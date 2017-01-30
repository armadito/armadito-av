      
var token = null;
      
function register() {
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            var obj = JSON.parse(xmlhttp.responseText);
            token = obj.token;
            console.log("token is now: " + token);
        }
    };
    xmlhttp.open("GET", "/api/register", true);
    xmlhttp.send(null);
}

function scan() {
    console.log("scan");
    var path_to_scan = document.getElementById("path").value;
    console.log("path to scan: " + path_to_scan);
    var data = {path: path_to_scan};
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.open("POST", "/api/scan", true);
    xmlhttp.setRequestHeader("X-Armadito-Token", token);
    xmlhttp.setRequestHeader("Content-Type", "application/json");
    xmlhttp.send(JSON.stringify(data));
    
    long_polling();
}

function status() {
    console.log("status");
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET", "/api/status", true);
    xmlhttp.setRequestHeader("X-Armadito-Token", token);
    xmlhttp.send(null);
    
    long_polling();
}

function long_polling() {
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
            var ev = JSON.parse(xmlhttp.responseText);
            if (ev.type == "EVENT_ON_DEMAND_PROGRESS") {
		document.getElementById("progress").innerHTML = ev.u.ev_on_demand_progress.progress;
		document.getElementById("current_path").innerHTML = ev.u.ev_on_demand_progress.path;
		document.getElementById("malware_count").innerHTML = ev.u.ev_on_demand_progress.malware_count;
		document.getElementById("suspicious_count").innerHTML = ev.u.ev_on_demand_progress.suspicious_count;
		document.getElementById("scanned_count").innerHTML = ev.u.ev_on_demand_progress.scanned_count;

		long_polling();
            } else if (ev.type == "EVENT_DETECTION") {
		var results = document.getElementById("results")
		var row = results.insertRow(1);
		var path = row.insertCell(0);
		path.innerHTML = ev.u.ev_detection.path;
		var status = row.insertCell(1);
		status.innerHTML = ev.u.ev_detection.scan_status;
		var action = row.insertCell(2);
		action.innerHTML = ev.u.ev_detection.scan_action;
		var module = row.insertCell(3);
		module.innerHTML = ev.u.ev_detection.module_name;
		var module_report = row.insertCell(4);
		module_report.innerHTML = ev.u.ev_detection.module_report;

		long_polling();
            } else if (ev.event_type == "StatusEvent") {
		document.getElementById("status").innerHTML = ev.global_status;
		document.getElementById("update_timestamp").innerHTML = ev.global_update_timestamp;
            }
        }
    };
    console.log("sending request");
    xmlhttp.open("GET", "/api/event", true);
    xmlhttp.setRequestHeader("X-Armadito-Token", token);
    xmlhttp.send(null);
}

function fun() {
    console.log("submit");
}
