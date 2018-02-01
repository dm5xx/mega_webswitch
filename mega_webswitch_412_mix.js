var jQueryScriptOutputted = false;

var textbank0 = ["Ant1","Ant2","Ant3","Ant4","Ant5","Ant6","Ant7","Ant8","Ant9","Ant10","Ant11","Ant12","OFF"];
var textbank1 = ["Ant1","Ant2","Ant3","Ant4","Ant5","Ant6","Ant7","Ant8","Ant9","Ant10","Ant11","Ant12","OFF"];
var textbank2 = ["Ant1","Ant2","Ant3","Ant4","Ant5","Ant6","Ant7","Ant8","Ant9","Ant10","Ant11","Ant12","OFF"];
var textbank3 = ["Ant1","Ant2","Ant3","Ant4","Ant5","Ant6","Ant7","Ant8","Ant9","Ant10","Ant11","Ant12","OFF"];

var togglebank0 = [[0,1,2,3,4,5,6,7,8,9,10,11]];
var togglebank1 = [[0,1,2,3,4,5,6,7,8,9,10,11]];
var togglebank2 = [[0,1,2,3,4,5,6,7,8,9,10,11]];
var togglebank3 = [[0,1,2,3,4,5,6,7,8,9,10,11]];

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

var secToken = 33196680;  // must be the same as in the ino definition
var excludeBankFromSlotLock = [ 1,0,0,1 ];  // must be the same as in the ino definition

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
    						}}, autoRefreshInSeconds * 10000);        
	
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

function isSlotLocked(b, s)
{
	var temp = 0;
	
	if (b == 0)
	{
	    if(excludeBankFromSlotLock[1] == 0)
    		temp = temp + parseInt(window["but_"+1+'_'+s]);
	    if (excludeBankFromSlotLock[2] == 0)
	        temp = temp + parseInt(window["but_" + 2 + '_' + s]);
	    if (excludeBankFromSlotLock[3] == 0)
	        temp = temp + parseInt(window["but_" + 3 + '_' + s]);
	}
	if (b == 1)
	{
	    if (excludeBankFromSlotLock[0] == 0)
    		temp = temp + parseInt(window["but_"+0+'_'+s]);
	    if (excludeBankFromSlotLock[2] == 0)
	        temp = temp + parseInt(window["but_" + 2 + '_' + s]);
	    if (excludeBankFromSlotLock[3] == 0)
    		temp = temp + parseInt(window["but_"+3+'_'+s]);
	}
	if (b === 2)
	{
	    if (excludeBankFromSlotLock[1] == 0)
	        temp = temp + parseInt(window["but_" + 1 + '_' + s]);
	    if (excludeBankFromSlotLock[0] == 0)
	        temp = temp + parseInt(window["but_" + 0 + '_' + s]);
	    if (excludeBankFromSlotLock[3] == 0)
	        temp = temp + parseInt(window["but_" + 3 + '_' + s]);
	}
	if (b === 3)
	{
	    if (excludeBankFromSlotLock[1] == 0)
	        temp = temp + parseInt(window["but_" + 1 + '_' + s]);
	    if (excludeBankFromSlotLock[2] == 0)
	        temp = temp + parseInt(window["but_" + 2 + '_' + s]);
	    if (excludeBankFromSlotLock[0] == 0)
	        temp = temp + parseInt(window["but_" + 0 + '_' + s]);
	}
		
	if(temp > 0)
		return true;
	return false;
}

function createBanks(banknumber)
{		
	for(var i = 0; i < 13; i++)
	{
		if(i<12)
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
		}
		else
		{
			var e = $('<div class="relayButton" onClick="updateButton_BankOff('+banknumber+')" onmouseover="mouseIsOver()" onmouseout="mouseIsOut()">All Off</div>');
			var buttonname = 'off_'+banknumber;

			e.addClass("rounded-corners");
			e.attr('id', buttonname);
			
			$('#bank'+banknumber).append(e);		
			
			var onbut = document.getElementById(buttonname);
			onbut.className = onbut.className + ' ball';
		}
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
	for(var i = 0; i < 12; i++)
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
	
	if (excludeBankFromSlotLock[bank] === 0 && isSlotLocked(bank, port)) {
	    alert("This slot is locked!");
		return;
	}

	if (excludeBankFromSlotLock[bank] === 0) { // big fun... :P this is the one-by one logic, because all buttons defined as depending to each other (see var definition)
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
    }

	toogleButton(bank, port, po); // toggle the original requested port

	window['bank'+bank+'BS'] = recreateBinaryStringForBank(bank);
	window['bank'+bank+'Data'] = bin2dec(eval('bank'+bank+'BS'));
	
	setContent(bank, window['bank'+bank+'Data']).done(function(data)
	{getAllContent();}
	);
}

function updateButton_BankOff(bank)
{	
	setContent(bank, 0).done(function(data)
	{ getAllContent(); }
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
