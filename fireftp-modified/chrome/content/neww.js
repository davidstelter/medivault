function temp() {

var prompts = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);

var items = ["cert1", "cert2", "cert3", "cert4"]; // list items

var selected = {};

var result = prompts.select(null, "Select Cert", "Please choose a certificate", items.length,items, selected);

var index = selected.value;

alert("Good!! \n"+ items[index] + '.');

}
