
// Demo implementation of an HTML progress bar, updated from JS
function avancement() {
  var progress = document.getElementById("scan_progress");
  var prc = document.getElementById("pourcentage");
  prc.innerHTML = progress.value + "%";
}

avancement(); //Initialisation

function modif(val) {
  var progress = document.getElementById("scan_progress");
  if((progress.value+val)<=progress.max && (progress.value+val)>=0) {
     progress.value += val;
  }
  avancement();
}