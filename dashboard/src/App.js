import React, { useEffect, useState } from 'react';

function App() {
  const [files, setFiles] = useState([]);
  const [content, setContent] = useState("");
  const [selectedFile, setSelectedFile] = useState(null);

  // 1. Fetch the list of files from your Node.js Bridge (Port 5000)
  const fetchFiles = () => {
    fetch('http://localhost:5000/files')
      .then(res => res.json())
      .then(data => setFiles(data))
      .catch(err => console.error("Bridge is not running:", err));
  };

  useEffect(() => {
    fetchFiles();
  }, []);

  // 2. Fetch the content of a specific C++ file
  const viewFile = (name) => {
    setSelectedFile(name);
    fetch(`http://localhost:5000/files/${name}`)
      .then(res => res.text())
      .then(text => setContent(text));
  };

  return (
    <div style={styles.container}>
      <header style={styles.header}>
        <h1>☁️ ASHUTOSH CLOUD MONITOR v1.0</h1>
        <button onClick={fetchFiles} style={styles.refreshBtn}>Refresh Explorer</button>
      </header>

      <div style={styles.main}>
        {/* Sidebar: File List */}
        <aside style={styles.sidebar}>
          <h3 style={{color: '#888'}}>FILE EXPLORER</h3>
          {files.length === 0 && <p>No files uploaded yet...</p>}
          {files.map(f => (
            <div 
              key={f} 
              onClick={() => viewFile(f)} 
              style={{...styles.fileItem, backgroundColor: selectedFile === f ? '#333' : 'transparent'}}
            >
              📄 {f.split('_')[1] || f}
              <div style={styles.timestamp}>{new Date(parseInt(f.split('_')[0])).toLocaleString()}</div>
            </div>
          ))}
        </aside>

        {/* Code Viewer */}
        <section style={styles.viewer}>
          <div style={styles.viewerHeader}>
            {selectedFile ? `Reading: ${selectedFile}` : "System Idle: Select a file to decrypt..."}
          </div>
          <pre style={styles.codeBlock}>
            {content || "// Waiting for selection..."}
          </pre>
        </section>
      </div>
    </div>
  );
}

// 🎨 Hacker/Dark Mode Styles
const styles = {
  container: { backgroundColor: '#0d1117', color: '#c9d1d9', minHeight: '100vh', fontFamily: "'Consolas', 'Courier New', monospace" },
  header: { padding: '20px', borderBottom: '1px solid #30363d', display: 'flex', justifyContent: 'space-between', alignItems: 'center' },
  refreshBtn: { backgroundColor: '#238636', color: 'white', border: 'none', padding: '10px 20px', borderRadius: '6px', cursor: 'pointer' },
  main: { display: 'flex', height: 'calc(100vh - 80px)' },
  sidebar: { width: '300px', borderRight: '1px solid #30363d', padding: '20px', overflowY: 'auto' },
  fileItem: { padding: '10px', marginBottom: '5px', borderRadius: '4px', cursor: 'pointer', transition: '0.2s' },
  timestamp: { fontSize: '0.7em', color: '#58a6ff', marginTop: '4px' },
  viewer: { flex: 1, display: 'flex', flexDirection: 'column' },
  viewerHeader: { padding: '10px 20px', backgroundColor: '#161b22', borderBottom: '1px solid #30363d', color: '#7d8590' },
  codeBlock: { margin: 0, padding: '20px', flex: 1, overflow: 'auto', color: '#9cdcfe', lineHeight: '1.5' }
};

export default App;