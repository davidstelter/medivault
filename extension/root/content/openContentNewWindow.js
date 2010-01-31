
function openStringInNewWindow(aString) {			
	var newWindow = window.open();			//opens a blank page
	newWindow.document.write(aString);
	newWindow.document.close();
}