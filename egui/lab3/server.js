'use strict';

const express = require('express');
const path    = require('path');
const app     = express();
const fs      = require('fs');

app.use(express.static(path.join(__dirname, 'build')));

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'build', 'index.html'));
});

app.get('/data', (req, res) => {
    res.sendFile(path.join(__dirname, 'data.json'));
});

app.post('/data', (req, res) => {
    req.pipe(fs.createWriteStream(path.join(__dirname, 'data.json')));
})

app.listen(8080);