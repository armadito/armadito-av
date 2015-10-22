var gui = require('nw.gui');

// New scan instance here
// Init connexion to AV
connect_to_AV();
ask_for_new_scan(global.new_scan);


// Demo implementation of an HTML progress bar, updated from JS
function avancement() {
  var progress = document.getElementById("scan_progress");
  var prc = document.getElementById("pourcentage");
  prc.innerHTML = progress.value + "%";
}

avancement(); //Initialisation

function update_scan_progress_bar (val) {
  var progress = document.getElementById("scan_progress");
  if((progress.value+val)<=progress.max && (progress.value+val)>=0) {
     progress.value += val;
  }
  avancement();
}