## Introduction

Node-webkit (NW) a été choisi pour développer l'interface graphique de l'Anti-Virus Desktop Windows (+ Linux/Mac OSX plus tard).
Ce webkit permet d'utiliser simplement les téchnologies web HTML5/CSS et JS (node.js, jquery, angularJS, etc). Mais surtout, NW.js supporte tous les 194000+ modules node.js.
Initié par IBM, cette technologie est fiable et multi-plateforme.

Node-webkit (NW) has been selected to be our Desktop AV IHM on Windows (+ Linux/Mac OSX later).
This webkit allows you to simply use new web technologies in a desktop application. Especially HTML5/CSS/JS (node.js, jquery, angularJS, etc).
Above all, NW.js support all existing node.js modules ( 194000+ node_modules). Launched by IBM, this technology is liable and multi-plateform compatible.

github : https://github.com/nwjs/nw.js

## Installation

To install and use the IHM implemented, you simply need to download the last version (0.12+) of nw.js for your operating system and architecture.
Once you have downloaded it, unzip the archive and move inside the new directory. You may need to install node.js and npm on the machine.

Some dependances are needed (node.js modules) :
- node-ipc

You can install it, by launching the following command in a cmd prompt or terminal :
npm install <module_name>  

For example: npm install node_ipc

Finally, you can run the current project by launching the following command in a cmd prompt or terminal :
<node_webkit_path>/<nw-binary> <project_path>

Note:
nw-binary can be "nw.exe" or nw.

## Packaging

// TODO