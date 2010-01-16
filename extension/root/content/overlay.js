var SPRS_Menus = {
  onLoad: function() {
    // initialization code
    this.initialized = true;
  },

  onMenuItemCommand: function() {
    window.open("chrome://secparest/content/hello.xul", "", "chrome");
  },
  onMenuCertsGenerate: function() {
    window.open("chrome://secparest/content/hello.xul", "", "chrome");
  },
  onMenuCertsView: function() {
    window.open("chrome://secparest/content/hello.xul", "", "chrome");
  },
  onMenuCertsSign: function() {
    window.open("chrome://secparest/content/hello.xul", "", "chrome");
  }
};

window.addEventListener("load", function(e) { SPRS_Menus.onLoad(e); }, false); 

