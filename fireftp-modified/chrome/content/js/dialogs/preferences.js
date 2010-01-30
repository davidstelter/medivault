function readPreferences() {
  window.sizeToContent();
  portChange();
  timeoutChange();
  filemodeChange();
  integrateChange();
  activePortChange();
}

function portChange() {
  $('proxyhost').disabled      =  $('proxytype').value == "";
  $('proxyport').disabled      =  $('proxytype').value == "";
  $('proxyhostlabel').disabled =  $('proxytype').value == "";
  $('proxyportlabel').disabled =  $('proxytype').value == "";
}

function timeoutChange() {
  $('retrylabel').disabled     = !$('timeoutmode').checked;
  $('retry').disabled          = !$('timeoutmode').checked;
  $('attemptslabel').disabled  = !$('timeoutmode').checked;
  $('attempts').disabled       = !$('timeoutmode').checked;
}

function filemodeChange() {
  $('asciibutton').disabled    =  $('filemode').value != 0;
}

function integrateChange() {
  $('temppasvmode').disabled   = !$('integrateftplinks').checked;
}

function activePortChange() {
  $('activelowportlabel').disabled  = !$('activeportmode').checked;
  $('activelowport').disabled       = !$('activeportmode').checked;
  $('activehighportlabel').disabled = !$('activeportmode').checked;
  $('activehighport').disabled      = !$('activeportmode').checked;
}
