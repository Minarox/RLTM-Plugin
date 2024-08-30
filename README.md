<div id="top"></div>
<br />

<div align="center">
<a href="https://github.com/Minarox/RLTM-Plugin">
    <img src="https://avatars.githubusercontent.com/u/71065703?v=4" alt="Logo Minarox" width="auto" height="80" style="border-radius: 4px">
</a>

<h3 align="center">RLTM Plugin</h3>

![Project version](https://img.shields.io/badge/Version-V1.0.0.458-blue)&nbsp;
![Project License](https://img.shields.io/github/license/Minarox/RLTM-Plugin?label=License)&nbsp;


  <p align="center">
    RLTM Plugin for BakkesMod
    <br />
    <a href="https://rltm.minarox.fr/"><strong>minarox.fr »</strong></a>
  </p>
</div>
<br />

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#features">Features</a></li>
        <li><a href="#tech-stack">Tech Stack</a></li>
      </ul>
    </li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#feedback">Feedback</a></li>
    <li><a href="#author">Author</a></li>
  </ol>
</details>

## About The Project

BakkesMod plugin to fetch and send real time data from Rocket League to the [RLTM Backend](https://github.com/Minarox/RLTM-Backend) through WebSocket.

### Features

- Connect and auto-reconnect to [Backend](https://github.com/Minarox/RLTM-Backend)
- Send match, statistics and entities datas
- Hide some parts of the HUD in spectator
- Remove Engine debug graph in spectator
- Automatically mark as ready at the end of the match
- Automatically save replay at the end of the match

### Tech Stack

- [socket.io-client-cpp](https://github.com/socketio/socket.io-client-cpp)
- [nlohmann/json](https://github.com/nlohmann/json)

<p align="right">(<a href="#top">back to top</a>)</p>

## Roadmap

- [x] WebSocket connection to [Backend](https://github.com/Minarox/RLTM-Backend)
- [x] Automatic reconnection on interruption
- [x] Fetch match data (time, score, state, etc.)
- [x] Fetch statistics data (scoreboard, statistic event, etc.)
- [x] Fetch entities data (boost, position, etc.)
- [ ] Fetch current spectator target
- [x] Send datas to [Backend](https://github.com/Minarox/RLTM-Backend)
- [ ] Optimize data stream with [protobuf](https://protobuf.dev/)
- [x] Hide HUD when spectating
- [x] Hide engine debug statistics when spectating (F10)
- [x] Mark as ready when match end
- [x] Save match replay when match end
- [ ] Customize replay name
- [ ] Plugin settings
- [ ] Control the game from [Backend](https://github.com/Minarox/RLTM-Backend)

<p align="right">(<a href="#top">back to top</a>)</p>

## Feedback

If you have any feedback, please reach out to us at [contact@minarox.fr](mailto:contact@minarox.fr).

<p align="right">(<a href="#top">back to top</a>)</p>

## Author

[@Minarox](https://www.github.com/Minarox)

<p align="right">(<a href="#top">back to top</a>)</p>
