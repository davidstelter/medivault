//Attempt to initialize the crypto hardware
function initCrypto(){
	if(acos5.SPRS_initCrypto()){
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
	}
	else{
		alert('Hardware initialization has failed, is your device connected?');
		//appendLog(gStrbundle.getString("wrap.initFailure")+": "+strings.getFormattedString("wrap.errorCode", [acos5.SPRS_getLastError()]));
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

