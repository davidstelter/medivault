//Attempt to initialize the crypto hardware
function initCrypto(){
	var status;
	if(status = acos5.SPRS_initCrypto()){
		var initSucc = gStrbundle.getString("wrap.initSuccess");
		appendLog(initSucc);
		var tokens = acos5.SPRS_getTokenCount();
		if(tokens == 0){
			msg = gStrbundle.getString("wrap.noTokens");
		}
		else{
			if(tokens == 1){
				msg = gStrbundle.getString("wrap.oneToken");
			}
			else{
				msg = gStrbundle.getFormattedString("wrap.nTokens", [tokens]);
			}
		}
		appendLog(msg);

		//user must select card now
		cardSelectDialog();
	}
	else{
		alert('Hardware initialization has failed, is your device connected?');
		//appendLog(gStrbundle.getString("wrap.initFailure")+": "+strings.getFormattedString("wrap.errorCode", [acos5.SPRS_getLastError()]));
	}
	return status;
}

function cardSelectDialog(){

	//Getting the slot  
	var prompts = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);

	//var items = ["cert1", "cert2", "cert3", "cert4"];
	var items = acos5.SPRS_enumerateCards({});

	var selected = {};

	var result = prompts.select(null, "Select Card", "Please choose a card", items.length,items, selected);
	if(!result)
		return;

	// sellected cert index
	var index = selected.value;


	//Getting the pin
	var pin = prompt("Please enter your PIN");
	// loading the slot
	var login = acos5.SPRS_selectCard(index, pin);
	//TODO: loop here to allow retries
	if(login)
		appendLog("Login successful");
	else{
		appendLog("Login failed!");
		finalizeCrypto();
	}
}

function finalizeCrypto(){
	acos5.SPRS_finalizeCrypto();
}

function reinitializeCrypto(){
	alert("reinitializing");
	finalizeCrypto();
	initCrypto();
}

