<div align="center">
  <img src="https://avatars.githubusercontent.com/u/71065703?v=4" alt="Logo" width="112px">

  <h1>WebSocket Controller Plugin</h1>

  Bi-directionnal communication between WebSocket server and Rocket League. <br />
  <a href="https://minarox.github.io/WSC-Plugin"><b>Documentation »</b></a>

  <sub>
    If you like this project, please star it & <a href="https://github.com/Minarox">follow me</a> to see what other cool projects I'm working on! ❤️
  </sub>
</div>

## ⭐️ Features

- 📃 **Data stream** - Send match, statistics and entities datas from the game in real time
- 🔍 **Ready for stream** - Hide some parts of the HUD and Engine debug graph in spectator
- ✅ **Shortcuts** - Automatically mark as ready and save replay file at the end of the match
- ⚙️ **Configurable** - Change target server, events, automatic actions and more

## 🛠️ Tech Stack

- [BakkesModSDK](https://github.com/bakkesmodorg/BakkesModSDK)
- [IXWebSocket](https://github.com/machinezone/IXWebSocket)
- [nlohmann/json](https://github.com/nlohmann/json)

## 🚗 Roadmap

- [x] WebSocket connection to server
- [x] Automatic reconnection on interruption
- [x] Fetch match data (time, score, state, etc.)
- [x] Fetch statistics data (scoreboard, statistic event, etc.)
- [x] Fetch entities data (boost, position, etc.)
- [x] Fetch players data (uid, name, car, etc.)
- [ ] Fetch current spectator target
- [x] Send datas to server
- [x] Hide HUD when spectating
- [x] Hide engine debug statistics when spectating (F10)
- [x] Mark as ready when match end
- [x] Save match replay when match end
- [x] Join spectator team on new game
- [ ] Customize replay name
- [ ] Plugin settings
- [ ] Control the game from the server
- [ ] Documentation

## 💼 License

Apache 2.0 © [Mathis Serrieres Maniecki](https://github.com/Minarox)
