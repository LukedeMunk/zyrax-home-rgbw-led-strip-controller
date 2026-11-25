/******************************************************************************/
/*
 * File:    strip.js
 * Author:  Luke de Munk
 * Version: 0.10.0
 *
 * Brief:   XXX
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
const powerControlsElem = document.querySelector("#power-controls");
const powerStatusElem = document.querySelector("#power-status");
const networkFormElem = document.querySelector("#network-form");

document.getElementById("strip-title").textContent = TEXT_STRIP_TITLE;
document.getElementById("power-title").textContent = TEXT_POWER;
document.getElementById("power-on").textContent = TEXT_POWER_ON;
document.getElementById("power-off").textContent = TEXT_POWER_OFF;
document.getElementById("configure-network").textContent = TEXT_CONFIGURE_NETWORK;
document.getElementById("ssid-label").textContent = TEXT_NETWORK_NAME;
document.getElementById("password-label").textContent = TEXT_WIFI_PASSWORD;
document.getElementById("hostname-label").textContent = TEXT_HOSTNAME;
document.getElementById("save-network").textContent = TEXT_SAVE_AND_RESTART;

powerControlsElem.addEventListener("click", async event => {
    const button = event.target.closest("button");
    if (!button) return;
    
    const buttons = powerControlsElem.querySelectorAll("button");
    buttons.forEach(item => item.disabled = true);

    try {
        await postCommand("set_power", new URLSearchParams({power: button.value}));
        powerStatusElem.textContent = button.value === "1" ? TEXT_POWER_ON_RECEIVED : TEXT_POWER_OFF_RECEIVED;
    } catch {
        powerStatusElem.textContent = TEXT_POWER_FAILED;
    } finally {
        buttons.forEach(item => item.disabled = false);
    }
});

networkFormElem.addEventListener("submit", async event => {
    event.preventDefault();
    const submit = networkFormElem.querySelector("button");
    const status = document.querySelector("#network-status");
    submit.disabled = true;

    try {
        await postCommand("configure_network", new URLSearchParams(new FormData(networkFormElem)));
        status.textContent = TEXT_NETWORK_SAVED;
    } catch {
        status.textContent = TEXT_NETWORK_SAVE_FAILED;
        submit.disabled = false;
    }
});

async function postCommand(command, body) {
    const response = await fetch("/api/v1/" + command, {method: "POST", body});
    if (!response.ok) throw new Error("Request failed");
}
