const express = require('express');
const fs = require('fs');
const path = require('path');
const cors = require('cors');

const app = express();
app.use(cors());

// This points to your C++ Server folder
const STORAGE_PATH = "C:\\Users\\ashut\\OneDrive\\Documents\\SystemsLabServer\\build\\Desktop_Qt_6_8_3_MinGW_64_bit-Debug\\debug";

app.get('/files', (req, res) => {
    fs.readdir(STORAGE_PATH, (err, files) => {
        if (err) return res.status(500).json({ error: "Cannot read folder" });
        const cloudFiles = files.filter(file => file.includes('_'));
        res.json(cloudFiles);
    });
});

app.get('/files/:name', (req, res) => {
    const filePath = path.join(STORAGE_PATH, req.params.name);
    fs.readFile(filePath, 'utf8', (err, data) => {
        if (err) return res.status(404).send("File not found");
        res.send(data);
    });
});

app.listen(5000, () => console.log("🚀 Bridge live on http://localhost:5000"));