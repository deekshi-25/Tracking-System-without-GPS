const socket = io("http://127.0.0.1:5000");

// ===== CALIBRATION =====
const DIST_START_CP1 = 1;
const DIST_CP1_CP2   = 1;
const DIST_CP2_END   = 1;   // NEW

// ===== STATE =====
let currentDistance = 0;
let cp1Reached = false;
let cp2Reached = false;
let endReached = false;

// ===== LIVE DISTANCE =====

socket.on("iot/vehicle/live", (data) => {
  document.getElementById("battery").innerText ="90%"; // Placeholder for battery status
  currentDistance = parseFloat(data);

  document.getElementById("dist").innerText =
    currentDistance.toFixed(2);

  // START → CP1
  if (!cp1Reached) {

    let progress1 =
      Math.min((currentDistance / DIST_START_CP1) * 100, 100);

    document.getElementById("fillLine").style.width =
      progress1 + "%";
  }

  // CP1 → CP2
  else if (cp1Reached && !cp2Reached) {

    let progress2 =
      Math.min((currentDistance / DIST_CP1_CP2) * 100, 100);

    document.getElementById("fillLine2").style.width =
      progress2 + "%";
  }

  // CP2 → END
  else if (cp2Reached && !endReached) {

    let progress3 =
      Math.min((currentDistance / DIST_CP2_END) * 100, 100);

    document.getElementById("fillLine3").style.width =
      progress3 + "%";
  }
});

// ===== CHECKPOINT EVENTS =====
socket.on("iot/vehicle/data", (data) => {

  document.getElementById("status").innerText =
    "Reached " + data.cp;

  // CP1
  if (data.cp === "CP1") {

    cp1Reached = true;

    document.getElementById("cp1").style.background = "green";
    document.getElementById("fillLine").style.width = "100%";
    document.getElementById("fillLine2").style.width = "0%";
  }

  // CP2
  if (data.cp === "CP2") {

    cp2Reached = true;

    document.getElementById("cp2").style.background = "green";
    document.getElementById("fillLine2").style.width = "100%";
    document.getElementById("fillLine3").style.width = "0%";
  }

  // END POINT (FINAL)
  if (data.cp === "END") {

    endReached = true;

    document.getElementById("end").style.background = "green";
    document.getElementById("fillLine3").style.width = "100%";

    // ✅ NO CARD VERIFICATION — AUTO COMPLETE
    document.getElementById("status").innerText =
      "Journey Completed ✅";

    // Optional: lock UI or stop updates
    socket.off("iot/vehicle/live");
  }
});

// ===== BIN LEVEL =====
socket.on("iot/vehicle/binlevel", (data) => {
  if (data == "0"){
    document.getElementById("binlevel").style.background = "green";
    document.getElementById("binlevel").innerText ="Bin can be filled";
    return;
  }
  else if(data == "1"){
    document.getElementById("binlevel").style.background = "red";
    document.getElementById("binlevel").innerText ="Bin is full";
    return;
  }
});