var SysTrayX = {};

SysTrayX.SaveOptions = {
  start: function(e) {
    e.preventDefault();

    console.debug("Save preferences");

    //
    // Save accounts and filters
    //

    console.debug("Store accounts and filters");

    const treeBase = document.getElementById("accountsTree");
    const inputs = treeBase.querySelectorAll("input");
    let accounts = [];
    let filters = [];
    for (let i = 0; i < inputs.length; ++i) {
      const account = JSON.parse(inputs[i].value);
      const checked = inputs[i].checked;
      accounts.push({ ...account, checked: checked });

      if (checked) {
        let inboxMailFolder = account.folders.find(obj => obj.type === "inbox");

        if (inboxMailFolder) {
          console.debug("Filter Id: " + inboxMailFolder.accountId);
          console.debug("Filter Path: " + inboxMailFolder.path);

          filters.push({
            unread: true,
            folder: inboxMailFolder
          });
        }
      }
    }

    //  Store accounts
    browser.storage.sync.set({
      accounts: accounts
    });

    //  Store query filters
    browser.storage.sync.set({
      filters: filters
    });

    console.debug("Store accounts and filters done");

    //
    //  Save debug state
    //
    let debug = document.querySelector('input[name="debug"]').checked;
    browser.storage.sync.set({
      debug: `${debug}`
    });
    console.debug("Store debug state: " + `${debug}`);

    //
    //  Save minimize hide state
    //
    let minimizeHide = document.querySelector('input[name="minimizeHide"]').checked;
    browser.storage.sync.set({
      minimizeHide: `${minimizeHide}`
    });
    console.debug("Store minimizeHide state: " + `${minimizeHide}`);

    //
    // Save icon preferences
    //

    console.debug("Store icon preferences");

    const iconType = document.querySelector('input[name="iconType"]:checked')
      .value;

    //  Store icon type
    browser.storage.sync.set({
      iconType: iconType
    });

    let iconDiv = document.getElementById("icon");
    let iconBase64 = iconDiv.getAttribute("data-icon");
    let iconMime = iconDiv.getAttribute("data-icon-mime");

    //  Store icon (base64)
    browser.storage.sync.set({
      iconMime: iconMime,
      icon: iconBase64
    });

    console.debug("Store icon preferences done");

    //  Mark add-on preferences changed
    browser.storage.sync.set({
      addonprefchanged: true
    });
  }
};

SysTrayX.RestoreOptions = {
  start: function() {
    console.debug("Restore preferences");

    console.debug("Restore icon preferences");

    //
    //  Restore debug state
    //
    const getDebug = browser.storage.sync.get("debug");
    getDebug.then(
      SysTrayX.RestoreOptions.setDebug,
      SysTrayX.RestoreOptions.onDebugError
    );

    //
    //  Restore minimize hide
    //
    const getMinimizeHide = browser.storage.sync.get("minimizeHide");
    getMinimizeHide.then(
      SysTrayX.RestoreOptions.setMinimizeHide,
      SysTrayX.RestoreOptions.onMinimizeHideError
    );

    //
    //  Restore icon type
    //
    const getIconType = browser.storage.sync.get("iconType");
    getIconType.then(
      SysTrayX.RestoreOptions.setIconType,
      SysTrayX.RestoreOptions.onIconTypeError
    );

    //
    //  Restore icon
    //
    const getIcon = browser.storage.sync.get(["iconMime", "icon"]);
    getIcon.then(
      SysTrayX.RestoreOptions.setIcon,
      SysTrayX.RestoreOptions.onIconError
    );

    console.debug("Restore icon preferences done");
  },

  //
  //  Restore debug state callbacks
  //
  setDebug: function(result) {
    const debug = result.debug || "false";

    console.debug("Debug: " + debug);

    const checkbox = document.querySelector(`input[name="debug"]`);
    checkbox.checked = debug === "true";
  },

  onDebugError: function(error) {
    console.log(`Debug Error: ${error}`);
  },

  //
  //  Restore minimize hide callbacks
  //
  setMinimizeHide: function(result) {
    const minimizeHide = result.minimizeHide || "true";

    console.debug("MinimizeHide: " + minimizeHide);

    const checkbox = document.querySelector(`input[name="minimizeHide"]`);
    checkbox.checked = minimizeHide === "true";
  },

  onMinimizeHideError: function(error) {
    console.log(`MinimizeHide Error: ${error}`);
  },

  //
  //  Restore icon type callbacks
  //
  setIconType: function(result) {
    const iconType = result.iconType || "0";
    const radioButton = document.querySelector(`[value="${iconType}"]`);
    radioButton.checked = true;
  },

  onIconTypeError: function(error) {
    console.log(`Icon type Error: ${error}`);
  },

  //
  //  Restore icon
  //
  setIconMime: function(result) {
    const iconMime = result.iconMime || "";

    const valid = iconMime !== "";
    if (valid) {
      const iconDiv = document.getElementById("icon");
      iconDiv.setAttribute("data-icon-mime", iconMime);
    }

    return valid;
  },

  setIconData: function(result) {
    const iconBase64 = result.icon || "";

    const valid = iconBase64 !== "";
    if (valid) {
      const iconDiv = document.getElementById("icon");
      iconDiv.setAttribute("data-icon", iconBase64);
    }

    return valid;
  },

  updateIconImage: function() {
    const iconDiv = document.getElementById("icon");
    icon_mime = iconDiv.getAttribute("data-icon-mime");
    icon_data = iconDiv.getAttribute("data-icon");

    const image = document.getElementById("customIconImage");
    image.setAttribute("src", `data:${icon_mime};base64,${icon_data}`);
  },

  setIcon: function(result) {
    const validMime = SysTrayX.RestoreOptions.setIconMime(result);
    const validData = SysTrayX.RestoreOptions.setIconData(result);

    if (validMime && validData) {
      SysTrayX.RestoreOptions.updateIconImage();
    }
  },

  onIconError: function(error) {
    console.log(`Icon Error: ${error}`);
  }
};

SysTrayX.StorageChanged = {
  changed: function(changes, area) {
    //  Try to keep the preferences of the add-on and the app in sync
    const changedItems = Object.keys(changes);

    let changed_icon = false;
    let changed_icon_mime = false;
    for (let item of changedItems) {
      if (item === "iconMime") {
        SysTrayX.RestoreOptions.setIconMime({
          iconMime: changes[item].newValue
        });
      }
      if (item === "icon") {
        SysTrayX.RestoreOptions.setIcon({ icon: changes[item].newValue });
        changed_icon = true;
      }
      if (item === "iconType") {
        SysTrayX.RestoreOptions.setIconType({
          iconType: changes[item].newValue
        });
        changed_icon_mime = true;
      }
      if (item === "minimizeHide") {
        SysTrayX.RestoreOptions.setMinimizeHide({
          minimizeHide: changes[item].newValue
        });
      }
      if (item === "debug") {
        SysTrayX.RestoreOptions.setDebug({
          debug: changes[item].newValue
        });
      }
    }

    if (changed_icon_mime && changed_icon) {
      SysTrayX.RestoreOptions.updateIconImage();
    }

    //
    //  Update element
    //
    document.getElementById("debugselect").className = "active";
    document.getElementById("iconselect").className = "active";
  }
};

//
//  Main
//
document.addEventListener("DOMContentLoaded", SysTrayX.RestoreOptions.start);
document
  .querySelector('[name="saveform"]')
  .addEventListener("submit", SysTrayX.SaveOptions.start);

browser.storage.onChanged.addListener(SysTrayX.StorageChanged.changed);
