window.setInterval("UIUpdate()", 500);                                             // update once a second

function UIUpdate() {
  if (gLogQueue && gLogMode) {
    var scrollLog = gCmdlogBody.scrollTop + 50 >= gCmdlogBody.scrollHeight - gCmdlogBody.clientHeight;
    gCmdlogBody.innerHTML += gLogQueue;                                             // update log

    gLogQueue = "";

    var nodeList = gCmdlogDoc.getElementsByTagName("div");                          // don't keep too much log data or it will slow down things
    var count    = 0;
    while (nodeList.length > 200 + count) {
      if (nodeList.item(count).getAttribute("type") == 'error') {
        ++count;
      } else {
        gCmdlogBody.removeChild(nodeList.item(count));
      }
    }

    if (scrollLog) {
      gCmdlogBody.scrollTop = gCmdlogBody.scrollHeight - gCmdlogBody.clientHeight;  // scroll to bottom
    }
  }

  queueTree.updateView();                                                           // update queue

  var bytesTotal;                                                                   // update status bar
  var bytesTransferred;
  var bytesPartial;
  var timeStart;

  for (var x = 0; x < gMaxCon; ++x) {
    if (!gConnections[x].isConnected) {
      continue;
    }

    if (gConnections[x].dataSocket && gConnections[x].dataSocket.progressEventSink.bytesTotal && !gConnections[x].dataSocket.progressEventSink.compressStream) {
      bytesTotal       = gConnections[x].dataSocket.progressEventSink.bytesTotal;
      bytesTransferred = gConnections[x].dataSocket.progressEventSink.bytesUploaded;
      bytesPartial     = gConnections[x].dataSocket.progressEventSink.bytesPartial;
      timeStart        = gConnections[x].dataSocket.progressEventSink.timeStart;
      break;
    } else if (gConnections[x].dataSocket && gConnections[x].dataSocket.dataListener.bytesTotal) {
      bytesTotal       = gConnections[x].dataSocket.dataListener.bytesTotal;
      bytesTransferred = gConnections[x].dataSocket.dataListener.bytesDownloaded;
      bytesPartial     = gConnections[x].dataSocket.dataListener.bytesPartial;
      timeStart        = gConnections[x].dataSocket.dataListener.timeStart;
      break;
    }
  }

  if (bytesTotal) {
    gStatusBarClear   = false;
    var timeElapsed   = ((new Date()) - timeStart) / 1000;
    timeElapsed       = timeElapsed != 0 ? timeElapsed : 1;                         // no dividing by 0
    var averageRate   = ((bytesTransferred - bytesPartial) / 1024 / timeElapsed).toFixed(2);
    averageRate       = averageRate != 0 ? averageRate : "0.1";                     // no dividing by 0
    var timeRemaining = (bytesTotal - bytesTransferred) / 1024 * (1 / averageRate);
    averageRate       = averageRate.replace(/\./g, gStrbundle.getString("decimal")) + " " + gStrbundle.getString("kbsec");
    var filesleft     = 0;

    for (var x = 0; x < gMaxCon; ++x) {
      if (!gConnections[x].isConnected) {
        continue;
      }

      var filesleftTrans = 0;

      if (gConnections[x].dataSocket && gConnections[x].dataSocket.progressEventSink.bytesTotal) {
        filesleftTrans = gConnections[x].dataSocket.progressEventSink.bytesUploaded;
      } else if (gConnections[x].dataSocket && gConnections[x].dataSocket.dataListener.bytesTotal) {
        filesleftTrans = gConnections[x].dataSocket.dataListener.bytesDownloaded;
      }

      filesleft += filesleftTrans ? 1 : 0;
    }

    for (var x = 0; x < gMaxCon; ++x) {
      if (!gConnections[x].isConnected) {
        continue;
      }

      for (var y = 0; y < gConnections[x].eventQueue.length; ++y) {
        if (gConnections[x].eventQueue[y].cmd in {"RETR": 1, "APPE": 1, "STOR": 1}) {
          ++filesleft;
        }

        if (filesleft > 100) {                  // save on CPU cycles, going through entire eventQueue can be intense
          filesleft = "100+";
          break;
        }
      }
    }

    gStatusBytes.label     = commas(bytesTransferred) + " / " + commas(bytesTotal) + ' - ' + gStrbundle.getFormattedString("filesleft", [filesleft]);
    var hours              = parseInt( timeElapsed / 3600);
    var min                = parseInt((timeElapsed - hours * 3600) / 60);
    var sec                = parseInt( timeElapsed - hours * 3600 - min * 60);
    gStatusElapsed.label   = zeros(hours) + ":" + zeros(min) + ":" + zeros(sec);

    hours                  = parseInt( timeRemaining / 3600);
    min                    = parseInt((timeRemaining - hours * 3600) / 60);
    sec                    = parseInt( timeRemaining - hours * 3600 - min * 60);
    gStatusRemaining.label = zeros(hours) + ":" + zeros(min) + ":" + zeros(sec);

    gStatusRate.label      = averageRate;
    var total              = bytesTotal != 0 ? bytesTotal : 1;                      // no dividing by 0
    var progress           = parseInt(bytesTransferred / total * 100) + "%";
    gStatusMeter.setAttribute("mode", "determined");
    gStatusMeter.setAttribute("value", progress);
    document.title         = progress + " @ " + averageRate + " - " + (gAccount ? gAccount : gFtp.host) + " - FireFTP";

  } else {
    var filesleft = 0;                                                              // update status bar to list how many files are left
    var status    = "";

    for (var x = 0; x < gMaxCon; ++x) {
      if (!gConnections[x].isConnected) {
        continue;
      }

      for (var y = 0; y < gConnections[x].eventQueue.length; ++y) {
        if (gConnections[x].eventQueue[y].cmd in {"RETR": 1, "RETR2": 1, "APPE": 1, "APPE2": 1, "STOR": 1, "STOR2": 1,
                                                  "DELE": 1, "RMD": 1, "get": 1, "reget": 1, "put": 1, "reput": 1}) {
          ++filesleft;
        }

        if (filesleft > 100) {                  // save on CPU cycles, going through entire eventQueue can be intense
          filesleft = "100+";
          break;
        }
      }
    }

    if (filesleft) {
      status = gStrbundle.getString("working") + ' - ' + gStrbundle.getFormattedString("filesleft", [filesleft]);
      gStatusMeter.setAttribute("mode", "undetermined");
      gStatusBarClear = false;
    } else if (gFtp.eventQueue.length) {
      status = gFtp.eventQueue[0].cmd == "welcome" ? gStrbundle.getString("connecting") : gStrbundle.getString("working");
      gStatusMeter.setAttribute("mode", "undetermined");
      gStatusBarClear = false;
    } else if (gProcessing) {
      status = gStrbundle.getString("working");
      gStatusMeter.setAttribute("mode", "undetermined");
      gStatusBarClear = false;
    } else if (!gStatusBarClear) {
      gStatusMeter.setAttribute("mode", "determined");
      gStatusBarClear = true;
    } else if (gStatusBarClear && !gFtp.isReconnecting) {
      return;
    }

    if (!gFtp.isReconnecting && !gFtp.isConnected && !$('abortbutton').disabled) {
      $('abortbutton').disabled = true;
    }

    if (gFtp.isReconnecting) {
      if (gFtp.reconnectsLeft) {
        status = gStrbundle.getFormattedString("reconnect", [gFtp.reconnectInterval, gFtp.reconnectsLeft]);
        gStatusMeter.setAttribute("mode", "undetermined");
      } else {
        status = "";
        gStatusMeter.setAttribute("mode", "determined");
      }
    }

    gStatusBytes.label = status;

    if (!gFtp.isConnected) {
      document.title = (gFtp.isReconnecting ? (status + " - ") : "") + "FireFTP";
      gStatusBarClear = false;
    } else {
      document.title = status + (status == "" ? "" : " - ") + (gAccount ? gAccount : gFtp.host) + " - FireFTP";
    }

    gStatusElapsed.label   = "";
    gStatusRemaining.label = "";
    gStatusRate.label      = "";
    gStatusMeter.setAttribute("value", "0%");
  }
}
