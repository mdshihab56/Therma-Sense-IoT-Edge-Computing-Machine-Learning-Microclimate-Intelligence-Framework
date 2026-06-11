import os
import pickle
import pandas as pd
import numpy as np
import requests
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import matplotlib
matplotlib.use('Agg')  
import matplotlib.pyplot as plt
app = Flask(__name__)
CORS(app)
API_KEY = "YOUR_OPENWEATHERMAP_API_KEY_HERE"  
CITY_NAME = "Dhaka"
DHAKA_FALLBACK_TEMP = 33.5
DHAKA_FALLBACK_HUMID = 74.0
with open('models/ann_model.pkl', 'rb') as f: ann_model = pickle.load(f)
with open('models/regressor.pkl', 'rb') as f: regressor_model = pickle.load(f)
@app.route('/')
def home(): return send_from_directory('.', 'index.html')
@app.route('/api/upload', methods=['POST'])
def upload_telemetry_file():
    if 'file' not in request.files: return jsonify({"error": "No file chunk found"}), 400
    file = request.files['file']
    if file.filename == '': return jsonify({"error": "No selected file name"}), 400
    try:
        df = pd.read_csv(file, encoding='utf-8-sig')
        df.columns = df.columns.str.strip().str.lower()
        if df.empty:
            return jsonify({"error": "The uploaded telemetry file contains no data rows."}), 400
        df.to_csv('data.csv', index=False)
        print(f"💾 [Flask Ingestion Sync] File saved. Loaded rows count: {len(df)}")
        return jsonify({"status": "success", "total_rows": len(df)})
    except Exception as e:
        return jsonify({"error": f"Corrupt structural file matrix: {e}"}), 400
@app.route('/api/predict', methods=['POST'])
def parse_csv_line():
    data = request.get_json()
    idx = data.get('row_index', 0)    
    if not os.path.exists('data.csv'): return jsonify({"error": "Upload a telemetry file first!"}), 400        
    df = pd.read_csv('data.csv')
    total_available_rows = len(df)   
    if idx >= total_available_rows or idx < 0:
        idx = total_available_rows - 1     
    in_temp = float(df.iloc[idx]['indoor_temp'])
    in_hum = float(df.iloc[idx]['indoor_humidity'])    
    dataset_avg_temp = float(df['indoor_temp'].mean())
    dataset_avg_hum = float(df['indoor_humidity'].mean())    
    out_temp, out_hum = DHAKA_FALLBACK_TEMP, DHAKA_FALLBACK_HUMID
    if API_KEY != "paste your api key":
        try:
            url = f"http://api.openweathermap.org/data/2.5/weather?q={CITY_NAME}&appid={API_KEY}&units=metric"
            weather_data = requests.get(url, timeout=3).json()
            out_temp = float(weather_data['main']['temp'])
            out_hum = float(weather_data['main']['humidity'])
        except Exception as e:
            print(f"⚠️ Live Weather Stream offline: {e}")

    
    input_vector = np.array([[in_temp, in_hum, out_temp, out_hum]])
    ann_prediction = int(ann_model.predict(input_vector)[0])
    status_label = "Optimal Thermal Comfort" if ann_prediction == 0 else "High Microclimatic Heat Stress Risk"
    predicted_trend = float(regressor_model.predict(input_vector)[0])

  
    fig, (ax1, ax3) = plt.subplots(2, 1, figsize=(12, 8))
    plt.rcParams['font.sans-serif'] = 'Arial'
    
   
    indices = np.arange(3) 
    width = 0.25
    
    
    temp_dataset = [in_temp, dataset_avg_temp, out_temp]
    color_temp = '#0071e3'
    ax1.bar(indices - width/2, temp_dataset, width, label='Temperature (°C)', color=color_temp, alpha=0.85, edgecolor='#00458b')
    ax1.set_ylabel('Temperature Scale (°C)', color=color_temp, fontweight='bold', fontsize=11)
    ax1.tick_params(axis='y', labelcolor=color_temp)
    ax1.set_ylim(0, 50)
    
    
    humid_dataset = [in_hum, dataset_avg_hum, out_hum]
    color_hum = '#34c759'
    ax2 = ax1.twinx()
    ax2.bar(indices + width/2, humid_dataset, width, label='Relative Humidity (%)', color=color_hum, alpha=0.85, edgecolor='#1a7f37')
    ax2.set_ylabel('Relative Humidity (%)', color=color_hum, fontweight='bold', fontsize=11)
    ax2.tick_params(axis='y', labelcolor=color_hum)
    ax2.set_ylim(0, 100)
    
    
    ax1.axhspan(20, 25, color='#34c759', alpha=0.12, label='Medical Comfort Threshold (20-25°C)')
    ax1.axhline(y=30.0, color='#ff453a', linestyle='--', linewidth=1.2, label='Clinical Heat Stress Alert (30°C)')
    
    ax1.set_xticks(indices)
    ax1.set_xticklabels([f'Target Row Index ({idx})', f'Bulk Run Mean ({total_available_rows} Rows Data)', 'Dhaka Macro-Climate Core (API)'], fontweight='bold', fontsize=10)
    ax1.set_title('Multi-Vector Environmental Discrepancy Matrix vs Human Health Standards', fontsize=13, fontweight='bold', pad=15)
    
    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper left', frameon=True, facecolor='#ffffff')
    ax1.grid(True, linestyle=':', alpha=0.6)
    ax2.grid(False)

    
    time_horizons = ['Current Ingest', '+1 Hour Target', '+2 Hours Target', '+3 Hours Target', '+4H Horizon']
    temp_curve = [in_temp, in_temp + (predicted_trend - in_temp)*0.35, in_temp + (predicted_trend - in_temp)*0.68, in_temp + (predicted_trend - in_temp)*0.88, predicted_trend]
    
    ax3.plot(time_horizons, temp_curve, marker='o', markersize=7, linestyle='-', color='#ff9500', linewidth=3, label='Random Forest Projected Trend Line')
    ax3.fill_between(time_horizons, temp_curve, color='#ff9500', alpha=0.08)
    
    ax3.axhline(y=35.0, color='#cf222e', linestyle='-.', linewidth=1.5, label='Extreme Clinical Hyperthermia Boundary (35°C)')
    ax3.axhline(y=25.0, color='#1a7f37', linestyle=':', linewidth=1.2, label='Upper Normal Physiological Bounds (25°C)')
    
    ax3.set_ylabel('Temperature Value Scale (°C)', fontweight='bold', fontsize=11)
    ax3.set_title('Machine Learning Proportional Temperature Forecast Profile (+4 Hour Multiplier)', fontsize=12, fontweight='bold', pad=12)
    ax3.set_ylim(min(temp_curve)-5, max(temp_curve)+6)
    ax3.legend(loc='lower left', frameon=True)
    ax3.grid(True, linestyle=':', alpha=0.6)

    plt.tight_layout()
    
    os.makedirs('static', exist_ok=True)
    chart_path = 'static/dashboard_analytics.png'
    plt.savefig(chart_path, dpi=180) 
    plt.close()

    return jsonify({
        "indoor_temp": in_temp, "indoor_humidity": in_hum,
        "outdoor_temp": out_temp, "outdoor_humidity": out_hum,
        "status": status_label, "prediction": round(predicted_trend, 2),
        "total_rows_parsed": total_available_rows,
        "chart_url": f"/{chart_path}?v={np.random.rand()}"
    })

@app.route('/static/<path:path>')
def serve_static(path): return send_from_directory('static', path)

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=5000, debug=True)