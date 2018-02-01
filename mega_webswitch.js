var jQueryScriptOutputted = false;

var textbank0 = ["HLAVNI PWR","3525 kHz","3600 kHz","3700 kHz","3775 kHz","OMP1-[5s]","OFF-1s","OPR","OMP2-[5s]","OFF-1s","OPR","ALPHA-ON-[5s]","OFF-1s","OPR","EMTRON-ON","OPR"];
var textbank1 = ["AL80b","H8K","VN-4.2-ON-1s","VN-4.2-OFF-1s","2x400","2xGU81","4xGU81","1xGU81","SB1000","MBst-ON-[5s]","OFF-1s","Velkoz-ON-1s","OFF-1s","L4B-1","L4B-2","L4B-3"];
var textbank2 = ["QRO1","QRO2","mH-20-ON-[5s]","OFF-1s","VN-5-ON-1s","OFF-1s","vH-20v-ON-[5s]","OFF-1s","vH-20z-ON-1s","OFF-1s","MLA","AL80A","MB2-ON-[5s]","OFF-1s","MB3-ON-[5s]","OFF-1s"];
var textbank3 = ["QRO1","QRO2","mH-20-ON-[5s]","OFF-1s","VN-5-ON-1s","OFF-1s","vH-20v-ON-[5s]","OFF-1s","vH-20z-ON-1s","OFF-1s","MLA","AL80A","MB2-ON-[5s]","OFF-1s","MB3-ON-[5s]","ServerOK"];

var togglebank0 = [[1,2,3,4],[6,7],[9,10],[12,13]];
var togglebank1 = [[]];
var togglebank2 = [[]];
var togglebank3 = [[]];

var bank0Data = 0;
var bank1Data = 0;
var bank2Data = 0;
var bank3Data = 0;
var bank0BS = '';
var bank1BS = '';
var bank2BS = '';
var bank3BS = '';

var ajaxArduinoUrl = configAddress;
var autoRefreshInSeconds = 3;
var mobileMultiplier = 3;
var isMouseOver = false;
var isMobile = false;

var secToken = 33196680;

$(document).ready(function() {
   init();
});

function init(){
	if((navigator.userAgent).indexOf("Mobile") > -1)
	{
		isMobile = true;
		autoRefreshInSeconds *= mobileMultiplier;
	}

    window.setInterval(function(){ 
    						if(isMobile || !isMouseOver) 
    						{ 
    							getAllContent();
    						}}, autoRefreshInSeconds * 1000);        
	
    if(isMobile)
    {
	    window.setInterval(function(){
	    	if(isMouseOver)
	    	{
	    		location.reload();
	    	}
	    }, 1000);
    }

	for (var j = 0; j < 4; j++)
	{
		window['bank'+j+'BS'] = dec2bin(window['bank'+j+'Data']);
		createButtonVars(j);
		createBanks(j);
	}

	getAllContent();

	for(var t = 0; t < 4; t++)
	{
		$('#greenBullet'+t).hide();
	}
};

var myResponseHandler = function(data) {

	var completeContent = [];
	completeContent = data.split('|');
	console.log(data);

	for(var t = 0; t < 4; t++)
	{
		$('#greenBullet'+t).show();
	}

	for(var h = 0; h < 4; h++)
	{
		window['bank'+h+'Data'] = parseInt(completeContent[h]);
		window['bank'+h+'BS'] = dec2bin(window['bank'+h+'Data']);
	  	createButtonVars(h);
	  	reloadBank(h);
	}

	setTimeout(function(){ 	
	for(var t = 0; t < 4; t++)
	{
		$('#greenBullet'+t).hide();
	}}, 500);
}

function mouseIsOver()
{
	isMouseOver = true;
}

function mouseIsOut()
{
	isMouseOver = false;
}

function createButtonVars(j)
{
		var revertedString = revertBinaryString(window['bank'+j+'BS']);
		for(var i = 0; i < 16 ; i++)
		{
			window["but_"+j+'_'+i] = revertedString[i];
		}
}

function createBanks(banknumber)
{		
	for(var i = 0; i < 16; i++)
	{
		var e = $('<div class="relayButton" onClick=updateButton('+banknumber+','+i+') onmouseover="mouseIsOver()" onmouseout="mouseIsOut()">'+eval('textbank'+banknumber+'['+i+']')+'</div>');
		var buttonname = 'b_'+banknumber+'_'+i;

		e.addClass("rounded-corners");
		e.attr('id', buttonname);
		
		$('#bank'+banknumber).append(e);

		
		var expo = eval('but_'+banknumber+'_'+i);

		if(expo == 1)
		{
			var onbut = document.getElementById(buttonname);
			onbut.className = onbut.className + ' bon';
		};
			
		if(expo == 0)
		{
			var offbut = document.getElementById(buttonname);
			offbut.className = offbut.className + ' boff';
		};
	};

}

function setButtonColor(mybuttonname, expo)
{
		var onbut = document.getElementById(mybuttonname);

		if(expo == 1)
		{
			onbut.className = "relayButton rounded-corners bon";
		}

		if(expo == 0)
		{
			onbut.className = "relayButton rounded-corners boff";
		}
		$("#"+mybuttonname).hide().show();
}

function reloadBank(abanknumber)
{
	for(var i = 0; i < 16; i++)
	{
		var buttonname = 'b_'+abanknumber+'_'+i;		
		var expo = eval('but_'+abanknumber+'_'+i);
		setButtonColor(buttonname, expo);		
	};
}

function updateButton(bank, port)
{		
	var po = eval('but_'+bank+'_'+port);
	var spectogglebank = eval('togglebank'+bank);

	console.log(port);
	
	for (var i = 0; i < spectogglebank.length; i++) { // toggle ports of a possible pair in a bank
		var innnerspecArray = spectogglebank[i];
		console.log(innnerspecArray);
		if($.inArray(port, innnerspecArray) > -1)
		{
			for (var j = 0; j < innnerspecArray.length; j++) {
				toogleButton(bank, innnerspecArray[j], 1);	// emulate 1 and switch them of than!					
			};
		}
	};
	toogleButton(bank, port, po); // toggle the original requested port

	window['bank'+bank+'BS'] = recreateBinaryStringForBank(bank);
	window['bank'+bank+'Data'] = bin2dec(eval('bank'+bank+'BS'));
	
	setContent(bank, window['bank'+bank+'Data']).done(function(data)
	{getAllContent();}
	);
}

function toogleButton(abank, aport, apo)
{
		var buttonname = 'b_'+abank+'_'+aport;
		var onbut = document.getElementById(buttonname);

		if(apo == 1)
		{
			onbut.className = "relayButton rounded-corners boff";
			window['but_'+abank+'_'+aport] = 0;				
		}

		if(apo == 0)
		{
			onbut.className = "relayButton rounded-corners bon";
			window['but_'+abank+'_'+aport] = 1;
		}
}



function recreateBinaryStringForBank(banknr)
{
	var rightBS = '';
	for(var b = 15; b >= 0; b--)
	{
		var myval =  eval('but_'+banknr+'_'+b);
		rightBS += myval;
	}

	return rightBS;
}

function getParameter(theParameter) 
{ 
  var params = window.location.search.substr(1).split('&');
 
  for (var i = 0; i < params.length; i++) {
    var p=params[i].split('=');
	if (p[0] == theParameter) {
	  return decodeURIComponent(p[1]);
	}
  }
  return false;
}

function dec2bin(dec)
{
	return pad(dec.toString(2), 16);  
}

function pad(n, width, z) 
{
	var z = z || '0';
	var n = n + '';
	return n.length >= width ? n : new Array(width - n.length + 1).join(z) + n;
}

function bin2dec(theString)
{
	return parseInt(theString, 2);
}

function revertBinaryString(aString)
{
	var returnValue = '';

	for (var a = 15; a >= 0; a--)
	{
		returnValue += aString[a];
	}
	return returnValue;
}

function getAllContent() {
		$.ajax({
			crossDomain: true,
			async : true,
			type: "GET",
			headers: {
				'Access-Control-Allow-Origin': '*'
			},
			url: ajaxArduinoUrl + "/GetAll/"+secToken+"?callback=?",
			dataType: 'jsonp',
			jsonpCallback:'xx'})
			.done(function(data){ 
				myResponseHandler(data.v);
			});
}

function setContent(currentBank, toBankValue) {
		jQuery.support.cors = true;
		
		
		return $.ajax({
			crossDomain: true,
			async : true,
			type: "GET",
			headers: {
				'Access-Control-Allow-Origin': '*'
			},
			url: ajaxArduinoUrl + "/Set/"+currentBank+"/"+toBankValue+"/"+secToken,
			contentType: "text/html",
			dataType: "html",
			success: function(data) {}
		});
}
