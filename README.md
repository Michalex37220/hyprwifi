**hyprwifi** is a lightweight graphical interface built with **C and GTK3** to manage Wi-Fi connections on Linux using the `nmcli` command-line tool (NetworkManager) for people who Network Manager f*cking broke.

---

## 🚀 Features

- 🔍 Scan and display nearby Wi-Fi networks
- 📋 Shows:
  - SSID (Network Name)
  - BSSID (MAC address)
  - Signal strength (%)
  - Security type (e.g. WPA2, open)
- 🔐 Connect to a selected network with a password prompt
- ❌ Disconnect from the current active Wi-Fi network
- 🌐 Select which network interface to use (e.g., `wlan0`, `wlp3s0`)
- ⚙️ Built on top of `nmcli`, ensuring wide compatibility

---

## 📦 Installation (Arch Linux)

You can build the package using `makepkg`:

```bash
git clone https://github.com/Michalex37220/hyprwifi
cd hyprwifi
makepkg -si
