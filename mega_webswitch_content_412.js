function addContent()
{
	var div = document.getElementById('content');
	console.log(div);
	div.innerHTML = '<div id="container"><div class="rotate" id="bb">Transceiver 1 <img id="greenBullet0" src="http://h.mmmedia-online.de/dot.png"></div><div id="bank0"></div></div>'+
					'<div id="container"><div class="rotate">Transceiver 2 <img id="greenBullet1" src="http://h.mmmedia-online.de/dot.png"></div><div id="bank1"></div></div>'+
					'<div id="container"><div class="rotate">Transceiver 3 <img id="greenBullet2" src="http://h.mmmedia-online.de/dot.png"></div><div id="bank2"></div></div>'+
					'<div id="container" class="myLast"><div class="rotate">Transceiver 4 <img id="greenBullet3" src="http://h.mmmedia-online.de/dot.png"></div><div id="bank3"></div></div>'+
					'<div id="tempResponse"></div>';
}
