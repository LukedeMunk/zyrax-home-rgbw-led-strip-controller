/******************************************************************************/
/*
 * File:    network_config_page.js
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Client code for configuring the wireless network of the controllers.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
//#region Elements
const networkConfigErrorMessageFieldElem = document.getElementById("networkConfigErrorMessageField");

const ssidTxtElem = document.getElementById("ssidTxt");
const passwordTxtElem = document.getElementById("passwordTxt");
const hostnameTxtElem = document.getElementById("hostnameTxt");
//#endregion

//#region Constants
const MAX_LENGTH_NAME = 50;
const MIN_LENGTH_PASSWORD = 8;
const MAX_LENGTH_PASSWORD = 64;

/* Text constants */
const TEXT_REQUIRED = "This field is required";
const TEXT_NO_SYMBOLS = "This field can not contain any symbols";
const TEXT_LENGTH = "At least 8 characters required";
const TEXT_INVALID_IP = "That IP is invalid";
const TEXT_CONFIGURATION_SAVED = "Network configuration saved. Controller is rebooting and connecting to the configured network.";
const TEXT_APPLY_CONFIG = "Are you sure that you want to apply the configuration?";
const TEXT_DISABLE_TRY = "Are you sure that the configuration is OK and that you want to disable trying? When there is an error in the configuration, the controller will try to connect infinitly. The only reason to disable trying is when the network is configured before the installation.";
const TEXT_Q_DISABLE_DHCP = "Are you sure that you want to configure this controller with a static IP setup? This is not recommended.";
const TEXT_Q_ARE_YOU_SURE = "Are you sure?";
const TEXT_TOO_LONG = "The text in this field is too long";
const TEXT_UPDATE = "Update";
const TEXT_SUCCESS = "Success";
const TEXT_CANCEL = "Cancel"
//#endregion

//#region Key event listeners
ssidTxtElem.addEventListener("keyup", function(event) {
    if (event.key === "Enter") {
        passwordTxtElem.focus();
    }
});
passwordTxtElem.addEventListener("keyup", function(event) {
    if (event.key === "Enter") {
        hostnameTxtElem.focus();
    }
});
hostnameTxtElem.addEventListener("keyup", function(event) {
    if (event.key === "Enter") {
        updateNetworkConfiguration();
    }
});
//#endregion

/******************************************************************************/
/*!
  @brief    When page finished loading, this function is executed.
*/
/******************************************************************************/
$(document).ready(function() {
    
});

//#region Configure network
/******************************************************************************/
/*!
  @brief    Shows the confirmation to update the network configuration.
*/
/******************************************************************************/
function updateNetworkConfigurationConfirm() {
    let buttons = [
                    {text: TEXT_UPDATE, onclickFunction: "updateNetworkConfiguration(true);"},
                    CANCEL_POPUP_BUTTON
                ];

    showPopup(TEXT_Q_ARE_YOU_SURE, TEXT_APPLY_CONFIG, buttons, BANNER_TYPE_WARNING)
}

/******************************************************************************/
/*!
  @brief    Updates the network configuration. Gets called multiple times until
            all confirmations are true.
  @param    updateConfirmed     If confirmed, true
*/
/******************************************************************************/
function updateNetworkConfiguration(updateConfirmed=false) {
    if (!validateInput()) {
        return;
    }

    var ssid = ssidTxtElem.value;
    var password = passwordTxtElem.value;
    var hostname = hostnameTxtElem.value;
    
    /* If not yet confirmed, show confirmation popup and return */
    if (!updateConfirmed) {
        updateNetworkConfigurationConfirm();
        return;
    } else {
        closePopup();
    }

    var data = {
        ssid: ssid,
        password: password,
        hostname: hostname
    }

    httpPostRequest("/configure_network", data);
    showBanner(TEXT_SUCCESS, TEXT_CONFIGURATION_SAVED, BANNER_TYPE_INFO);
}

/******************************************************************************/
/*!
  @brief    Validates the network configuration input.
*/
/******************************************************************************/
function validateInput() {
    /* Get user input */
    var ssid = ssidTxtElem.value;
    var password = passwordTxtElem.value;
    var hostname = hostnameTxtElem.value;
    
    /* Reset validation elements */
    networkConfigErrorMessageFieldElem.style.display = "none";

    ssidTxtElem.classList.remove("invalid-input");
    passwordTxtElem.classList.remove("invalid-input");
    hostnameTxtElem.classList.remove("invalid-input");
    
    /* Validate SSID */
    if (ssid == "") {
        ssidTxtElem.classList.add("invalid-input");
        ssidTxtElem.focus();
        networkConfigErrorMessageFieldElem.textContent = TEXT_REQUIRED;
        networkConfigErrorMessageFieldElem.style.display = "inline-block";
        return false;
    }

    if (ssid.match(SYMBOL_CRITICAL_RE)) {
        ssidTxtElem.classList.add("invalid-input");
        ssidTxtElem.focus();
        networkConfigErrorMessageFieldElem.textContent = TEXT_NO_SYMBOLS;
        networkConfigErrorMessageFieldElem.style.display = "inline-block";
        return false;
    }

    if (ssid.length > MAX_LENGTH_NAME) {
        ssidTxtElem.classList.add("invalid-input");
        ssidTxtElem.focus();
        networkConfigErrorMessageFieldElem.textContent = TEXT_TOO_LONG;
        networkConfigErrorMessageFieldElem.style.display = "inline-block";
        return false;
    }

    /* Validate password */
    if (password == "") {
        passwordTxtElem.classList.add("invalid-input");
        passwordTxtElem.focus();
        networkConfigErrorMessageFieldElem.textContent = TEXT_REQUIRED;
        networkConfigErrorMessageFieldElem.style.display = "inline-block";
        return false;
    }

    if (password.length < MIN_LENGTH_PASSWORD) {
        passwordTxtElem.classList.add("invalid-input");
        passwordTxtElem.focus();
        networkConfigErrorMessageFieldElem.textContent = TEXT_LENGTH;
        networkConfigErrorMessageFieldElem.style.display = "inline-block";
        return false;
    }

    if (password.length > MAX_LENGTH_PASSWORD) {
        passwordTxtElem.classList.add("invalid-input");
        passwordTxtElem.focus();
        networkConfigErrorMessageFieldElem.textContent = TEXT_TOO_LONG;
        networkConfigErrorMessageFieldElem.style.display = "inline-block";
        return false;
    }

    /* Validate hostname */
    if (hostname == "") {
        hostnameTxtElem.classList.add("invalid-input");
        hostnameTxtElem.focus();
        networkConfigErrorMessageFieldElem.textContent = TEXT_REQUIRED;
        networkConfigErrorMessageFieldElem.style.display = "inline-block";
        return false;
    }
    
    if (hostname.match(SYMBOL_ALL_RE)) {
        hostnameTxtElem.classList.add("invalid-input");
        hostnameTxtElem.focus();
        networkConfigErrorMessageFieldElem.textContent = TEXT_NO_SYMBOLS;
        networkConfigErrorMessageFieldElem.style.display = "inline-block";
        return false;
    }
    
    if (hostname.length > MAX_LENGTH_NAME) {
        hostnameTxtElem.classList.add("invalid-input");
        hostnameTxtElem.focus();
        networkConfigErrorMessageFieldElem.textContent = TEXT_TOO_LONG;
        networkConfigErrorMessageFieldElem.style.display = "inline-block";
        return false;
    }

    return true;
}
//#endregion

//#region Utilities
//#endregion