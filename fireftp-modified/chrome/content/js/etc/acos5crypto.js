function encryptFile(FileIn, FileOut, CertName){
	if(!checkForSession())
		return;

	appendLog("encrypting file "+FileIn+" with cert \""+CertName+"\", writing to "+FileOut);
	var result = acos5.SPRS_encryptFile(FileIn, FileOut, CertName);

	if(result){
		appendLog("Encrypt reports success");
	}
	else{
		appendLog("Encrypt failed!");
		logIfWrapperError();
	}
}

function signFile(FileIn, FileOut, CertName){
	if(!checkForSession())
		return;

	appendLog("signing file "+FileIn+" with cert \""+CertName+"\", writing to "+FileOut);
	var result = acos5.SPRS_signFile(FileIn, FileOut, CertName);

	if(result){
		appendLog("Sign reports success");
	}
	else{
		appendLog("Sign failed!");
		logIfWrapperError();
	}
}

function loadFile(FileIn){
	if(!checkForSession())
		return null;

	appendLog("loading file "+FileIn);

	var status;
	var clear = {};
	if(status = acos5.SPRS_loadFile(FileIn, clear)){
		//alert("Cleartext: " +clear.value);
	}
	else{
		appendLog("load file failed:");
		logWrapperError();
	}
	return clear.value;
}

function checkForSession(){
	if(!crypto_session_active)
		gPromptService.alert(null, "Error", "No active session with crypto hardware!");

	return crypto_session_active;
}

//Attempt to initialize the crypto hardware
function initCrypto(){
	var msg;
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
		crypto_session_active=true;
		appendLog(msg);

		//user must select card now
		cardSelectDialog();
	}
	else{
		crypto_session_active=false;
		gPromptService.alert(null, 'Error', 'Hardware initialization has failed, is your device connected?');
		logWrapperError();
		//appendLog(gStrbundle.getString("wrap.initFailure")+": "+strings.getFormattedString("wrap.errorCode", [acos5.SPRS_getLastError()]));
	}
	return status;
}


function finalizeCrypto(){
	crypto_session_active=false;
	acos5.SPRS_finalizeCrypto();
}

function reinitializeCrypto(){
	finalizeCrypto();
	initCrypto();
}

function logIfWrapperError(){
	var code = acos5.SPRS_getLastError();
	if(code != 0)
		appendLog(decodeWrapperError(code));
}

function logWrapperError(){
	var code = acos5.SPRS_getLastError();
		appendLog(decodeWrapperError(code));
}

function decodeWrapperError(code){
	switch (code){
		case  0: return "No error";
		case  1: return "DLL Load Error";
		case  2: return "No function list";
		case  3: return "Initialization failed";
		case  4: return "Already loaded";
		case  5: return "Open session failed";
		case  6: return "Key not found";
		case  7: return	"Invalid attribute";
		case  8: return "Could not init decryption";
		case  9: return "Failed to decrypt";
		case 10: return "No devices found";
		case 11: return "Failed to open file for reading";
		case 12: return "Failed to open file for writing";
		case 13: return "Failed to encrypt";
		case 14: return "Could not init encryption";
		case 15: return "Wrong password";
		case 16: return "Could not init signing";
		case 17: return "Failed to sign";
		case 18: return "Failed to read file";
		case 19: return "Could not init digest";
		case 20: return "Failed to digest";
		case 21: return "Failed to verify";
		default: return "Unknown error code "+code;
	}
}
