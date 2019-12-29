setInterval(readSensor, 2500);

function readSensor() {
	var req = new XMLHttpRequest();

	req.onreadystatechange = function () {
		if (this.readyState == 4 && this.status == 200) {
			document.getElementById("readings").innerHTML = req.responseText;
		}
	};

	req.open("GET", "read", true);
	req.send();
}