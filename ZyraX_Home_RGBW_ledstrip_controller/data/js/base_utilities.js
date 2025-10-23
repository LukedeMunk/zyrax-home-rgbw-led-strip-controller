/******************************************************************************/
/*
 * File:    base_utilities.js
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Global code that handles global utility functions. More information:
 *          https://github.com/LukedeMunk/zyrax-home-main-controller
 */
/******************************************************************************/
//#region Elements
/* Site wide elements */
const pageBackdropElem = document.getElementById("pageBackdrop");

/* Banners */
const bannerStack = document.getElementById("bannerStack");

/* Loading banner modal */
const loadingBannerElem = document.getElementById("loadingBanner");
const loadingBannerMessageFieldElem = document.getElementById("loadingBannerMessageField");
const loadingBannerProgressElem = document.getElementById("loadingBannerProgress");
const loadingBannerProgressBarElem = document.getElementById("loadingBannerProgressBar");

/* Popup modal */
const popupElem = document.getElementById("popup");
const popupIconElem = document.getElementById("popupIcon");
const popupTitleElem = document.getElementById("popupTitle");
const popupMessageFieldElem = document.getElementById("popupMessageField");
const popupButtonContainerElem = document.getElementById("popupButtonContainer");

/* Navigation bar */
const navigationBarContainerElem = document.getElementById("navigationBarContainer");
//#endregion

//#region Constants
/* Regular expressions */
const CHARACTER_RE = /^[^a-zA-Z]+$/;
const NUMBER_RE = /^[^0-9]+$/;
const SYMBOL_ALL_RE = /[-+!$%^&*()_|~=`{}\[\]:@#";'<>?,.\/\s]+/;
const SYMBOL_CRITICAL_RE = /[$*|~=\[\]:;#"'<>?,.\/]+/;
const SYMBOL_CRITICAL_WITH_POINTS_RE = /[$*|~=\[\]:;#"'<>?,\/]+/;
const IP_RE = /^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$/;
const DATE_RE = /^\d{2}\-\d{2}\-\d{4}$/;
const TIME_RE = /^\d{2}\:\d{2}$/;
const VERSION_RE = /^v([0-9]+)\_([0-9]+)\_([0-9]+)$/;
const PASSWORD_RE = /^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)(?=.*[#$^+=!*()@%&]).{8,64}$/;  //To check passwords according to NIST

/* Popup type classes */
const BANNER_TYPE_SUCCESS = "success";
const BANNER_TYPE_WARNING = "warning";
const BANNER_TYPE_ERROR = "error";
const BANNER_TYPE_INFO = "info";

/* HTTP codes */
const HTTP_CODE_OK = 200;
const HTTP_CODE_UNAUTHORIZED = 401;

/* Time delays */
const FETCH_TIME_OUT = 4000;
const SHOW_BANNER_TIME = 5000;
const UPDATE_INTERVAL_PAUSE_TIME = 2000;
const BACK_END_UPDATE_INTERVAL_1M = 60*1000;
const BACK_END_UPDATE_INTERVAL_10S = 10000;
const BACK_END_UPDATE_INTERVAL_5S = 5000;
const BACK_END_UPDATE_INTERVAL_1S = 1000;

const MOBILE_VERSION = screen.width < 700;
const CUSTOM_MOUSE_CONTEXT_MENU_ENABLED = true;

const LEFT_MOUSE_BUTTON = "click";
const RIGHT_MOUSE_BUTTON = "contextmenu";

const CANCEL_POPUP_BUTTON = {text: TEXT_CANCEL, onclickFunction: "closePopup();"};

//#region Language array
const LANGUAGES = [
    "Afrikaans",
    "Albanian - Shqip",
    "Arabic - العربية",
    "Belarusian - беларуская",
    "Bengali - বাংলা",
    "Bosnian - Bosanski",
    "Bulgarian - български",
    "Chinese - 中文",
    "Croatian - Hrvatski",
    "Czech - čeština",
    "Danish - Dansk",
    "Dutch - Nederlands",
    "English",
    "Esperanto - Esperanto",
    "Estonian - Eesti",
    "Finnish - Suomi",
    "French - Français",
    "German - Deutsch",
    "German (Switzerland) - Deutsch (Schweiz)",
    "Greek - Ελληνικά",
    "Hungarian - Magyar",
    "Icelandic - íslenska",
    "Indonesian - Indonesia",
    "Italian - Italiano",
    "Japanese - 日本語",
    "Korean - 한국어",
    "Latin",
    "Lithuanian - Lietuvių",
    "Macedonian - македонски",
    "Maltese - Malti",
    "Nepali - नेपाली",
    "Norwegian - Norsk",
    "Polish - Polski",
    "Portuguese - Português",
    "Punjabi - ਪੰਜਾਬੀ",
    "Romanian - Română",
    "Russian - Pусский",
    "Slovak - Slovenčina",
    "Slovenian - Slovenščina",
    "Somali - Soomaali",
    "Spanish - Español",
    "Swedish - Svenska",
    "Thai - ไทย",
    "Turkish - Türkçe",
    "Ukrainian - Yкраїнська",
    "Vietnamese - Tiếng Việt",
    "Western Frisian"
];

const LANGUAGE_ABBREVIATIONS = [
    "af",
    "sq",
    "ar",
    "be",
    "bn",
    "bs",
    "bg",
    "zh",
    "hr",
    "cs",
    "da",
    "nl",
    "en",
    "eo",
    "et",
    "fi",
    "fr",
    "de",
    "ds",
    "el",
    "hu",
    "is",
    "id",
    "it",
    "ja",
    "ko",
    "la",
    "lt",
    "mk",
    "mt",
    "ne",
    "no",
    "pl",
    "pt",
    "pa",
    "ro",
    "ru",
    "sk",
    "sl",
    "so",
    "es",
    "sv",
    "th",
    "tr",
    "uk",
    "vi",
    "fy"
];

function getLanguageByAbbriviation(abbreviation) {
    var index = 0;
    for (var languageAbbreviation of LANGUAGE_ABBREVIATIONS) {
        if (languageAbbreviation == abbreviation) {
            return index;
        }
        index++;
    }

    return NO_LANGUAGE;
}
//#endregion
//#endregion

//#region Variables
let fixedToActive = true;
let mouseContextMenuVisible = false;
//#endregion

//#region Disable scrolling variables
let scrollKeys = ["ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight"];
let wheelEvent = 'onwheel' in document.createElement('div') ? 'wheel' : 'mousewheel';

//Modern Chrome requires { passive: false } when adding event (for scrolling)
let supportsPassive = false;
try {
    window.addEventListener("test", null, Object.defineProperty({}, 'passive', {
        get: function() { supportsPassive = true; } 
    }));
} catch(e) {}
let wheelOpt = supportsPassive ? { passive: false } : false;
//#endregion

//#region Overlays
/******************************************************************************/
/*!
  @brief    Shows an overlay (for when the user interface is unavailable).
  @param    showLoadingCursor   When true, a loading cursor is shown on the
            overlay
*/
/******************************************************************************/
function showPageOverlay(showLoadingCursor=true) {
    if (showLoadingCursor) {
        pageBackdropElem.style.cursor = "wait";
    } else {
        pageBackdropElem.style.cursor = "default";
    }
    pageBackdropElem.style.display = "block";
    pageBackdropElem.style.opacity = 1;
}

/******************************************************************************/
/*!
  @brief    Hides the shown overlay.
*/
/******************************************************************************/
function hideOverlay() {
    pageBackdropElem.style.opacity = 0;

    /* After animation, close modal */
    setTimeout(() => {
        pageBackdropElem.style.display = "none";
    }, 300);
}
//#endregion

//#region HTTP requests
/******************************************************************************/
/*!
  @brief    Executes an HTTP request to the back-end without page refresh.
  @param    url                 Endpoint to request
  @param    data                Data to send in JSON format
  @param    nestedJson          When true, nested JSON is sended
  @param    postRequest         When true, a POST request is sended, else a GET
                                request is sended
*/
/******************************************************************************/
function httpRequest(url, data={}, postRequest=false, nestedJson=false) {
    console.log("Sending data to: " + url);
    console.log(data);

    var requestType = "get";
    if (postRequest) {
        requestType = "post";
    }
    
    let contentType= "application/x-www-form-urlencoded; charset=UTF-8";
    if (nestedJson) {
        data = JSON.stringify(data);
        contentType = "application/json; charset=utf-8";
    }
    
    $.ajax({
        url: url,
        type: requestType,
        data: data,
        contentType: contentType,
        success: function(response) {
            console.log(response);
        },
        error: function(xhr) {
            if (xhr.status == HTTP_CODE_UNAUTHORIZED) {
                localStorage.setItem("loggedIn", false);
                if (window.location.pathname != "/login_page") {
                    redirect("./login_page");
                }
            }

            showBanner(TEXT_SERVER_ERROR, TEXT_ERROR + ": " + xhr.status, BANNER_TYPE_ERROR);
        }
    });
}

/******************************************************************************/
/*!
  @brief    Executes an HTTP request to the back-end without page refresh and 
  @param    url                 Endpoint to request
  @param    data                Data to send in JSON format
  @param    nestedJson          When true, nested JSON is sended
*/
/******************************************************************************/
function httpPostRequest(url, data={}, nestedJson=false) {
    httpRequest(url, data, true, nestedJson);
}

/******************************************************************************/
/*!
  @brief    Executes an HTTP request to the back-end without page refresh and
            returns the JSON response.
  @param    url             Endpoint to request
  @param    data            Data to send in JSON format
  @param    postRequest     When true, a POST request is sended, else a GET
                            request is sended
  @param    async           When true, asynchronous function is used
  @param    nestedJson      When true, nested JSON is sended
*/
/******************************************************************************/
function httpRequestJsonReturn(url, data={}, postRequest=false, async=false, nestedJson=false) {
    console.log("Sending data to: " + url);
    console.log(data);
    
    let requestType = "get";
    if (postRequest) {
        requestType = "post";
    }

    let contentType= "application/x-www-form-urlencoded; charset=UTF-8";
    if (nestedJson) {
        data = JSON.stringify(data);
        contentType = "application/json; charset=utf-8";
    }

    let serverResponse = {};

    $.ajax({
        async: async,
        url: url,
        type: requestType,
        data: data,
        contentType: contentType,
        success: function(response) {
            serverResponse = JSON.parse(response);
        },
        error: function(response) {
            if (response.status == HTTP_CODE_UNAUTHORIZED) {
                if (window.location.pathname != "/login") {
                    redirect("./login");
                }
            } 

            serverResponse = {
                status_code: response.status,
                message: response.statusText + ", code: " + response.status
            }
        }
    });

    console.log("serverResponse:");
    console.log(serverResponse);
    return serverResponse;
}

/******************************************************************************/
/*!
  @brief    Executes an HTTP request to the back-end and returns the response.
  @param    url             Endpoint to request
  @param    data            Data to send in JSON format
  @param    async           When true, asynchronous function is used
  @param    nestedJson      When true, nested JSON is sended
*/
/******************************************************************************/
function httpPostRequestJsonReturn(url, data={}, async=false, nestedJson=false) {
    return httpRequestJsonReturn(url, data, true, async, nestedJson);
}
//#endregion

//#region Banner
/******************************************************************************/
/*!
  @brief    Generates a banner with the specified values.
  @param    title               Title to show
  @param    message             Message to show
  @param    onclickFunction     Function that gets executed when the user clicks
                                on the banner.
  @returns  DOM element         Banner DOM element
*/
/******************************************************************************/
function _generateBannerElement(title, message, onclickFunction=undefined) {
    const banner = document.createElement("div");
    banner.className = "banner-container";

    /* Onclick function */
    if (onclickFunction != undefined) {
        banner.style.cursor = "pointer";
        banner.setAttribute("onclick", onclickFunction);
    }

    /* Icon */
    const bannerIcon = document.createElement("i");
    bannerIcon.className = "banner-icon";

    /* Contentblok */
    const content = document.createElement("div");
    content.className = "banner-content";

    /* Title */
    const bannerTitle = document.createElement("p");
    bannerTitle.className = "banner-title";
    bannerTitle.textContent = title || "Notification";

    /* Message */
    const bannerMessage = document.createElement("p");
    bannerMessage.className = "banner-message";
    bannerMessage.textContent = message || "";

    content.appendChild(bannerTitle);
    content.appendChild(bannerMessage);

    /* Close button */
    const bannerClose = document.createElement("i");
    bannerClose.className = "fa-solid fa-xmark banner-close-button clickable";

    banner.appendChild(bannerIcon);
    banner.appendChild(content);
    banner.appendChild(bannerClose);
    
    return banner;
}

/******************************************************************************/
/*!
  @brief    Shows a banner with the specified values.
  @param    title               Title to show
  @param    message             Message to show
  @param    type                Type of the banner (info, warning, error, etc.)
  @param    timeout             Timeout to close the banner
  @param    onclickFunction     Function that gets executed when the user clicks
                                on the banner.
*/
/******************************************************************************/
function showBanner(title, message, type, timeout=SHOW_BANNER_TIME, onclickFunction=undefined) {
    const banner = _generateBannerElement(title, message, onclickFunction);
    const bannerIcon = banner.querySelector(".banner-icon");
    const bannerClose = banner.querySelector(".banner-close-button");
    bannerClose.addEventListener("click", () => closeBanner(banner));

    switch (type) {
        case BANNER_TYPE_INFO:
            bannerIcon.className = "banner-icon fa-solid fa-circle-info";
            bannerIcon.style.color = "var(--icon_blue)";
            break;
        case BANNER_TYPE_SUCCESS:
            bannerIcon.className = "banner-icon fa-solid fa-check-circle";
            bannerIcon.style.color = "var(--icon_green)";
            break;
        case BANNER_TYPE_WARNING:
            bannerIcon.className = "banner-icon fa-solid fa-triangle-exclamation";
            bannerIcon.style.color = "var(--icon_orange)";
            break;
        case BANNER_TYPE_ERROR:
            bannerIcon.className = "banner-icon fa-solid fa-circle-xmark";
            bannerIcon.style.color = "var(--icon_red)";
    }

    bannerStack.appendChild(banner);
    banner.getBoundingClientRect();                                             //Force the browser to calculate layout so the transition works
    banner.classList.add("show");                                               //Then add 'show' for the in-animation

    /* Auto-close */
    if (timeout > 0) {
        setTimeout(() => closeBanner(banner), timeout);
    }
}

/******************************************************************************/
/*!
  @brief    Closes the specified banner.
  @param    bannerElem          Banner element to close
*/
/******************************************************************************/
function closeBanner(bannerElem) {
    bannerElem.classList.remove("show");
    setTimeout(() => bannerElem.remove(), 300);
}
//#endregion

//#region Loading banner
/******************************************************************************/ 
/*!
  @brief    Shows a loading banner with a message, loading icon and optionally a
            progress bar.
  @param    message             Message to show
  @param    statusUrl           URL to get the loading status of
  @param    statusVariable      Variable to look for
  @param    successValue        Value that gets returned when finished
  @param    successTitle        Title to show when successful
  @param    successMessage      Message to show when successful
  @param    showProgressBar     If true, the progress bar will be shown
  @param    showOverlay         If true, the overlay will be shown
*/
/******************************************************************************/
function showLoadingBanner(message,
                            statusUrl = undefined,
                            statusVariable = undefined,
                            successValue = undefined,
                            successTitle = TEXT_SUCCESS,
                            successMessage = TEXT_SUCCESS,
                            showProgressBar = false,
                            showOverlay = false) {

    loadingBannerMessageFieldElem.textContent = message;

    /* Optionally show overlay behind banner */
    if (showOverlay) {
        showPageOverlay();
    }

    /* Show/hide progress bar */
    if (showProgressBar) {
        loadingBannerProgressElem.style.display = "block";
    } else {
        loadingBannerProgressElem.style.display = "none";
    }

    loadingBannerElem.getBoundingClientRect();                                  //Force the browser to calculate layout so the transition works
    loadingBannerElem.classList.add("show");                                    //Then add 'show' for the in-animation

    /* If provided, start polling the status URL */
    if (statusUrl != undefined) {
        setTimeout(function() {
            waitUntilFinished(statusUrl, statusVariable, successValue, 1000, successTitle, successMessage, showProgressBar);
        }, 500);
    }
}

/******************************************************************************/
/*!
  @brief    Closes the loading banner.
*/
/******************************************************************************/
function closeLoadingBanner() {
    loadingBannerElem.classList.remove("show");
    hideOverlay();
}

/******************************************************************************/
/*!
  @brief    Waits untill the system is done with the specified background task
            and shows the success message.
  @param    statusUrl           URL to get the loading status of
  @param    statusVariable      Variable to look for
  @param    successValue        Value that gets returned when finished
  @param    interval            Interval to check the status
  @param    successTitle        Title to show when successful
  @param    successMessage      Message to show when successfull
  @param    showProgressBar     If true, the progress bar will be shown
*/
/******************************************************************************/
async function waitUntilFinished(statusUrl,
                                statusVariable,
                                successValue,
                                interval=BACK_END_UPDATE_INTERVAL_1S,
                                successTitle=TEXT_SUCCESS,
                                successMessage=TEXT_SUCCESS,
                                showProgressBar=false) {
    try {
        var response = await fetch(statusUrl, {signal: AbortSignal.timeout(FETCH_TIME_OUT)});
    } catch {
        setTimeout(function() {
            waitUntilFinished(statusUrl, statusVariable, successValue, interval, successTitle, successMessage, showProgressBar);
        }, UPDATE_INTERVAL_PAUSE_TIME);

        return;
    }

    var data = await response.json();

    console.log(statusVariable)
    console.log(successValue)
    console.log(data)

    if (showProgressBar) {
        showProgress(data[statusVariable], loadingBannerProgressBarElem);
    }

    /* If is not ready, look again after some time */
    if (data[statusVariable] != successValue) {
        setTimeout(function() {
            waitUntilFinished(statusUrl, statusVariable, successValue, interval, successTitle, successMessage, showProgressBar);
        }, interval);

        return;
    }

    closeLoadingBanner();
    hideOverlay();
    setTimeout(() => {
        showBanner(successTitle, successMessage, BANNER_TYPE_SUCCESS);
    }, 100);
}

/******************************************************************************/
/*!
  @brief    Waits until the system is done with the specified background task
            and executes the specified function.
  @param    statusUrl           URL to get the loading status of
  @param    statusVariable      Variable to look for
  @param    successValue        Value that gets returned when finished
  @param    interval            Interval to check the status
  @param    successMessage      Message to show when successful
*/
/******************************************************************************/
async function waitUntilFinishedFunction(statusUrl, statusVariable, successValue, functionAfterFinished, interval=BACK_END_UPDATE_INTERVAL_1S) {
    try {
        var response = await fetch(statusUrl, {signal: AbortSignal.timeout(FETCH_TIMEOUT)});
    } catch {
        setTimeout(function() {
            waitUntilFinishedFunction(statusUrl, statusVariable, successValue, functionAfterFinished, interval);
        }, UPDATE_INTERVAL_PAUSE_TIME);

        return;
    }

    var data = await response.json();

    /* If is not ready, look again after some time */
    if (data[statusVariable] != successValue) {
        setTimeout(function() {
            waitUntilFinishedFunction(statusUrl, statusVariable, successValue, functionAfterFinished, interval);
        }, interval);

        return;
    }

    setTimeout(functionAfterFinished, 10);
}
//#endregion

//#region Popups
/******************************************************************************/
/*!
  @brief    Shows a popup.
  @param    title               Title of the confirmation
  @param    message             Confirmation message
  @param    buttons             Buttons array {text: [TEXT], onclickFunction: [FUNCTION]}
  @param    type                Type of the popup (info, warning, error, etc.)
*/
/******************************************************************************/
function showPopup(title, message, buttons, type=BANNER_TYPE_SUCCESS) {
    popupTitleElem.textContent = title;
    popupMessageFieldElem.textContent = message;

    popupIconElem.className = "popup-icon";
    switch (type) {
        case BANNER_TYPE_INFO:
            popupIconElem.className = "popup-icon fa-solid fa-circle-info";
            popupIconElem.style.color = "var(--icon_blue)";
            break;
        case BANNER_TYPE_SUCCESS:
            popupIconElem.className = "popup-icon fa-solid fa-check-circle";
            popupIconElem.style.color = "var(--icon_green)";
            break;
        case BANNER_TYPE_WARNING:
            popupIconElem.className = "popup-icon fa-solid fa-triangle-exclamation";
            popupIconElem.style.color = "var(--icon_orange)";
            break;
        case BANNER_TYPE_ERROR:
            popupIconElem.className = "popup-icon fa-solid fa-circle-xmark";
            popupIconElem.style.color = "var(--icon_red)";
            break;
    }

    popupButtonContainerElem.innerHTML = "";

    for (let button of buttons) {
        let buttonElem = document.createElement("button");
        buttonElem.textContent = button.text;
        buttonElem.setAttribute("onclick", button.onclickFunction);
        
        popupButtonContainerElem.appendChild(buttonElem);
    }

    showPageOverlay(false);
    popupElem.getBoundingClientRect();                                     //Force the browser to calculate layout so the transition works
    popupElem.classList.add("show");                                       //Then add 'show' for the in-animation
}

/******************************************************************************/
/*!
  @brief    Closes the popup.
*/
/******************************************************************************/
function closePopup() {
    popupElem.classList.remove("show");
    hideOverlay();
}
//#endregion

//#region Modals
/******************************************************************************/
/*!
  @brief    Shows the specified modal popup (with overlay).
  @param    modalElement    Modal element to show
*/
/******************************************************************************/
function showModal(modalElement) {
    disableScrolling();
    showPageOverlay(false);
    modalElement.showModal();
    modalElement.classList.add("show");
}

/******************************************************************************/
/*!
  @brief    Closes the specified modal popup.
  @param    modalElement    Modal element to close
  @param    hidePageOverlay When true, the overlay is hidden
*/
/******************************************************************************/
function closeModal(modalElement, hidePageOverlay=true) {
    enableScrolling();

    if (hidePageOverlay) {
        hideOverlay();
    }

    modalElement.classList.remove("show");

    /* After animation, close modal */
    setTimeout(() => {
        modalElement.close();
    }, 500);
}
//#endregion

//#region Utilities
//#region Loaders
/******************************************************************************/
/*!
  @brief    Generates close buttons for the popups.
*/
/******************************************************************************/
function loadModalCloseButtons() {
    var closeButtons = [...document.querySelectorAll(".close-modal-button")];   //Select all elements with classname close-modal-button

    /* For each element (closebutton), set which modal needs to be closed */
    closeButtons.forEach(function(button) {
        button.title = TEXT_CLOSE;
        button.onclick = function () {
            var modalId = button.getAttribute("target-modal");
            closeModal(document.getElementById(modalId));
        };
    });
}
//#endregion

//#region Mouse context menu
/******************************************************************************/
/*!
  @brief    Generates a mouse context menu with the specified items.
  @param    menuItems           Array of menu items
  @param    targetElement       Target DOM element
  @param    mouseButton         Mouse button to open menu
*/
/******************************************************************************/
function generateMouseContextMenu(menuItems, targetElement=undefined, mouseButton=RIGHT_MOUSE_BUTTON) {
    /*
     * Example
        [
            { text: "File", icon: "fa-solid fa-file", submenu: [
                { text: "New", icon: "fa-solid fa-plus", onclickFunction: "alert('New')" },
                { text: "Open", icon: "fa-solid fa-folder-open", onclickFunction: "alert('Open')" }
            ]},
            { text: "Delete", icon: "fa-solid fa-trash", onclickFunction: "alert('Delete')" }
        ]
     *
     * */
    if (targetElement == undefined) {
        targetElement = document;
    }
    
    /* Create menu */
    const menuElem = document.createElement("div");
    menuElem.className = "mouse-context-menu";
    document.body.appendChild(menuElem);

    let visible = false;
    let hideTimeout = null;
    let hideSubmenuTimeout = null;

    /* Recursive helper to create items (including submenus) */
    function createMenuItem(item, parentElem) {
        let itemContainer = document.createElement("div");
        itemContainer.className = "mouse-context-menu-item";

        let itemIcon = document.createElement("i");
        itemIcon.className = item.icon;

        let itemText = document.createElement("p");
        itemText.style.margin = "0px";
        itemText.textContent = item.text;
        
        let subItemIcon = document.createElement("i");
        subItemIcon.className = "fa-solid fa-caret-right";

        /* Assign onclick function if exists */
        if (item.onclickFunction) {
            itemContainer.setAttribute("onclick", item.onclickFunction);
        }

        itemContainer.appendChild(itemIcon);
        itemContainer.appendChild(itemText);

        /* Caret for submenu */
        if (item.submenu) {
            itemContainer.appendChild(subItemIcon);
        }
        
        parentElem.appendChild(itemContainer);

        /* If there is a submenu, create it recursively */
        if (item.submenu && Array.isArray(item.submenu)) {
            const submenuElem = document.createElement("div");
            submenuElem.className = "submenu";
            for (let subItem of item.submenu) {
                createMenuItem(subItem, submenuElem);
            }
            itemContainer.appendChild(submenuElem);

            /* Hover logic for submenu */
            itemContainer.addEventListener("mouseenter", () => {
                clearTimeout(hideSubmenuTimeout);
                submenuElem.classList.add("hover");
            });

            itemContainer.addEventListener("mouseleave", (e) => {
                hideSubmenuTimeout = setTimeout(() => {submenuElem.classList.remove("hover");}, 50);
            });
        }
    }

    /* Generate all top-level items */
    for (let item of menuItems) {
        createMenuItem(item, menuElem);
    }

    /* Show / hide helpers */
    function setPosition({ top, left }) {
        menuElem.style.left = `${left-2}px`;                                    //-2px, otherwise mouseleave event will be triggered when mouse moves
        menuElem.style.top = `${top-2}px`;
    }

    function show({ top, left }) {
        setPosition({ top, left });
        menuElem.classList.add("show");
        setTimeout(() => {visible = true;}, 10);                                //Delay, otherwise 'click' is triggered en menu is hide right away
    }

    function hide() {
        menuElem.classList.remove("show");
        visible = false;
    }

    function destroy() {
        hide();
        menuElem.remove();
        targetElement.removeEventListener(mouseButton, trigger);
    }

    /* Hover leave/enter */
    menuElem.addEventListener("mouseenter", () => {
        clearTimeout(hideTimeout);
    });

    menuElem.addEventListener("mouseleave", () => {
        hideTimeout = setTimeout(hide, 50);
    });

    /* Trigger events */
    function trigger(e) {
        e.preventDefault();
        show({ top: e.clientY, left: e.clientX });
    }
    
    targetElement.addEventListener(mouseButton, trigger);

    /* Global hide on click/scroll */
    window.addEventListener("click", () => { if (visible) hide(); });
    window.addEventListener("scroll", () => { if (visible) hide(); });

    return {show, hide, destroy, elem: menuElem};
}
//#endregion

//#region Scrolling
/******************************************************************************/
/*!
  @brief    Prevents the default action of the specified event.
  @param    e                   Event
*/
/******************************************************************************/
function preventDefault(e) {
    e.preventDefault();
}

/******************************************************************************/
/*!
  @brief    Prevents the default action of the specified event.
  @param    e                   Event
*/
/******************************************************************************/
function preventDefaultForScrollKeys(e) {
    /* When in text input area, user can use keys to navigate within the area */
    if (e.target.tagName == "TEXTAREA" || e.target.tagName == "INPUT") {
        return;
    }

    if (scrollKeys.includes(e.key)) {
        e.preventDefault();
        return false;
    }
}

/******************************************************************************/
/*!
  @brief    Disables page scrolling.
*/
/******************************************************************************/
function disableScrolling() {
    /* For now, mobile scrolling is allowed */
    if (MOBILE_VERSION) {
        return;
    }

    window.addEventListener('DOMMouseScroll', preventDefault, false);           //older FF
    window.addEventListener(wheelEvent, preventDefault, wheelOpt);              //modern desktop
    window.addEventListener('touchmove', preventDefault, wheelOpt);             //mobile
    window.addEventListener('keydown', preventDefaultForScrollKeys, false);
}

/******************************************************************************/
/*!
  @brief    Enables page scrolling.
*/
/******************************************************************************/
function enableScrolling() {
    /* For now, mobile scrolling is allowed */
    if (MOBILE_VERSION) {
        return;
    }

    window.removeEventListener('DOMMouseScroll', preventDefault, false);        //older FF
    window.removeEventListener(wheelEvent, preventDefault, wheelOpt);           //modern desktop
    window.removeEventListener('touchmove', preventDefault, wheelOpt);          //mobile
    window.removeEventListener('keydown', preventDefaultForScrollKeys, false);
}

/******************************************************************************/
/*!
  @brief    When modal is closed by the 'escape' key, enables page scrolling.
*/
/******************************************************************************/
document.body.addEventListener('keydown', function(e) {
    if (e.key == "Escape") {
        enableScrolling();
    }
});

/******************************************************************************/
/*!
  @brief    When unloading the page, can show an alert.
*/
/******************************************************************************/
$(window).on('beforeunload', function() {
    //
});
//#endregion

//#region Date and Time functionality
/******************************************************************************/
/*!
  @brief    X
  @returns  int                 X
*/
/******************************************************************************/
function getDateRangeOfWeek(weekNumber){
    var date = new Date();
    var numOfdaysPastSinceLastMonday = eval(date.getDay() - 1);

    date.setDate(date.getDate() - numOfdaysPastSinceLastMonday);

    var weekNoToday = date.getWeek();
    var weeksInTheFuture = eval(weekNumber - weekNoToday);

    date.setDate(date.getDate() + eval(7 * weeksInTheFuture));

    var dates = [];
    for (var day = 0; day < DAYS_IN_WEEK; day++) {
        dates.push(
            ('0' + date.getDate()).slice(-2) + "-"
            + ('0' + eval(date.getMonth()+1)).slice(-2) + "-"                   //Force format dd-mm-yyyy
            + date.getFullYear()
        );
        date.setDate(date.getDate() + 1);
    }

    return dates;
}

/******************************************************************************/
/*!
  @brief    X
  @returns  int                 X
*/
/******************************************************************************/
function getTimeStringFromMinutes(minutes) {
    var hourString = String(Math.floor(minutes / 60)).padStart(2, '0');
    var minuteString = String((minutes % 60)).padStart(2, '0');

    return hourString + ":" + minuteString;
}

/******************************************************************************/
/*!
  @brief    X
  @returns  int                 X
*/
/******************************************************************************/
Date.prototype.addDays = function(days) {
    var date = new Date(this.valueOf());
    date.setDate(date.getDate() + days);

    return date;
}

/******************************************************************************/
/*!
  @brief    X
  @returns  int                 X
*/
/******************************************************************************/
Date.prototype.subtractDays = function(days) {
    var date = new Date(this.valueOf());
    date.setDate(date.getDate() - days);

    return date;
}

/******************************************************************************/
/*!
  @brief    X
  @returns  int                 X
*/
/******************************************************************************/
Date.prototype.toDateString = function() {
    var dateTimeOffsetted = this.getTime();
    dateTimeOffsetted = new Date(dateTimeOffsetted + 2 * 60 * 60 * 1000);       //GMT+200

    var date = dateTimeOffsetted.toISOString().split('T')[0];
    date = reverseDateFormat(date);

    return date;
}

/******************************************************************************/
/*!
  @brief    X
  @returns  int                 X
*/
/******************************************************************************/
Date.prototype.getWeek = function() {
    var date = new Date(this.valueOf());                                        //Copy date so don't modify original
    date.setUTCDate(date.getUTCDate() + 4 - (date.getUTCDay() || 7));           //Set to nearest Thursday: current date + 4 - current day number. Make Sunday's day number 7

    var yearStart = new Date(Date.UTC(date.getUTCFullYear(), 0, 1));            //Get first day of year
    var weekNumber = Math.ceil((((date - yearStart) / 86400000) + 1) / 7);      //Calculate full weeks to nearest Thursday

    return weekNumber;
}
//#endregion

//#region Other
/******************************************************************************/
/*!
  @brief    Redirects to the specified URL.
  @param    url                 URL to redirect to
*/
/******************************************************************************/
function redirect(url) {
    window.location.href = url;
}

/******************************************************************************/
/*!
  @brief    Returns the file extension based on the filename.
  @param    filename            Name of the file
  @returns  string              Extension of the file
*/
/******************************************************************************/
function getFileExtension(filename) {
    var filenameArray = filename.split(".")
    return filename.split(".")[filenameArray.length-1];
}

/******************************************************************************/
/*!
  @brief    Calculates a gradient with the specified ratio.
  @param    ratio               Ratio color1 color2
  @param    color1              First color (hex)
  @param    color2              Second color (hex)
  @returns  string              Hex color string
*/
/******************************************************************************/
function getGradient(ratio, color1="329637", color2="328896") {
    var hex = function(x) {
      x = x.toString(16);
      return (x.length == 1) ? '0' + x : x;
    };
  
    var r = Math.ceil(parseInt(color1.substring(0,2), 16) * ratio + parseInt(color2.substring(0,2), 16) * (1-ratio));
    var g = Math.ceil(parseInt(color1.substring(2,4), 16) * ratio + parseInt(color2.substring(2,4), 16) * (1-ratio));
    var b = Math.ceil(parseInt(color1.substring(4,6), 16) * ratio + parseInt(color2.substring(4,6), 16) * (1-ratio));
    
    return '#' + hex(r) + hex(g) + hex(b);
}

/******************************************************************************/
/*!
  @brief    Shows the progress in the specified element.
  @param    percentage          Percentage to show
  @param    barElem             Element to show the progress in
*/
/******************************************************************************/
function showProgress(percentage, barElem) {
    barElem.style.width = percentage.toString() + "%";
    barElem.style.backgroundColor = getGradient(percentage / 100);
    barElem.textContent = Math.round(percentage).toString() + "%";

    if (percentage == 100) {
        setTimeout(function() {
            showProgress(0, barElem)
        }, 1000);
    }
}

/******************************************************************************/
/*!
  @brief    Returns the index of the specified specified ID
  @param    array               Array to look in
  @param    id                  ID to look for
  @param    idKey               If true, the array is a JSON object with ID key
  @returns  int                 Array index of the playlist
*/
/******************************************************************************/
function getIndexFromId(array, id, idKey=true) {
    for (let index in array) {
        if (idKey) {
            if (array[index].id == id) {
                return index;
            }
        } else {
            if (array[index] == id) {
                return index;
            }
        }
    }

    console.log("getIndexFromId Failed");
    return -1;
}
//#endregion

//#region Navigation bar
/******************************************************************************/
/*!
  @brief    Generates the navigation bar menu with the specified items.
  @param    menuItems           Array of menu items
*/
/******************************************************************************/
function generateNavigationBar(menuItems) {
    /*
     * Example
        [
            {pages: ["CONFIGURATION"], text: "Configuration", icon: "fa-duotone fa-sliders", onclickFunction: "alert('Test')"},
            {pages: ["AUTOMATIONS"], text: "Automations", link: "./automations", icon: "fa-duotone fa-calendar-week"},
            {pages: ["DASHBOARD"], text: "Dashboard", link: "./", icon: "fa-duotone fa-grid-horizontal"},
            {pages: ["CONFIGURATION"], text: "Configuration", link: "./configuration", icon: "fa-duotone fa-sliders"},
            {pages: ["CONFIGURATION"], text: "", link: "./configuration", icon: "fa-duotone fa-sliders"},
            {pages: ["CONFIGURATION"], text: "Configuration", icon: "fa-duotone fa-sliders", onclickFunction: "alert('Test')"}
        ];
     *
     * */
    navigationBarContainerElem.innerHTML = "";

    /* Build bar */
    let previousType = null;                                                    //Keep track of previous item's type

    let index = 0;
    for (let item of menuItems) {
        let element;
        let currentType = item.link ? "link" : item.onclickFunction ? "button" : null;

        /* Insert separator only if previous and current item are different types */
        if (previousType && currentType && previousType !== currentType) {
            let separator = document.createElement("div");
            separator.className = "separator";
            navigationBarContainerElem.appendChild(separator);
        }

        /* Create the actual element */
        if (item.link) {
            element = document.createElement("a");
            element.href = item.link;
            if (item.pages.includes(page)) {
                element.classList.add("selected");                              //Mark active
            }
        } else if (item.onclickFunction) {
            element = document.createElement("div");
            element.className = "button-item";
            element.setAttribute("onclick", item.onclickFunction);
        }

        if (item.icon) {
            let icon = document.createElement("i");
            icon.className = item.icon;
            element.appendChild(icon);
        }

        if (item.text) {
            let title = document.createElement("p");
            title.textContent = item.text;
            title.className = "navigation-item-title";
            element.appendChild(title);
        }

        navigationBarContainerElem.appendChild(element);

        /* Update previous type */
        previousType = currentType;
        index++;
    }
}
//#endregion
//#endregion