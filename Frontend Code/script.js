function copyFormattedCSVRow() {
  const rawInput = document.getElementById("serialInput").value.trim();
  const clipboardStatus = document.getElementById("clipboardStatus");

  if (!rawInput.includes(",") || rawInput.split(",").length !== 2) {
    alert(
      "Invalid formatting. Please input parameters matching this layout: Temperature,Humidity (e.g., 30.2,64.1)",
    );
    return;
  }

  navigator.clipboard
    .writeText(rawInput)
    .then(() => {
      clipboardStatus.style.display = "block";

      document.getElementById("serialInput").value = "";

      setTimeout(() => {
        clipboardStatus.style.display = "none";
      }, 4000);
    })
    .catch((err) => {
      alert("Clipboard access denied by browser configurations.");
    });
}

async function triggerAIDiagnostic() {
  const rowId = document.getElementById("rowIndex").value;
  try {
    const response = await fetch("/api/predict", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ row_index: parseInt(rowId) }),
    });
    if (!response.ok) throw new Error("Index range error");
    const data = await response.json();

    document.getElementById("inTemp").innerText = data.indoor_temp;
    document.getElementById("inHum").innerText = data.indoor_humidity;
    document.getElementById("outTemp").innerText = data.outdoor_temp;
    document.getElementById("predTrend").innerText = data.prediction;

    const statusBox = document.getElementById("statusOutput");
    statusBox.innerText = data.status;
    if (data.status.includes("Optimal")) {
      statusBox.style.backgroundColor = "#e2f6ea";
      statusBox.style.color = "#1a7f37";
    } else {
      statusBox.style.backgroundColor = "#ffebe9";
      statusBox.style.color = "#cf222e";
    }
    document.getElementById("chartContainer").innerHTML =
      `<img src="${data.chart_url}" class="fade-in">`;
  } catch (err) {
    alert(
      "Targeted CSV row index does not exist yet! Add more datasets from your file.",
    );
  }
}
