document.getElementById('compress-form').addEventListener('submit', async function(e) {
  e.preventDefault();

  const fileInput = document.querySelector('input[type="file"]');
  const file = fileInput.files[0];
  if (!file) return alert('Pick a file first');

  const arrayBuffer = await file.arrayBuffer();
  const compressed = pako.gzip(new Uint8Array(arrayBuffer));

  // trigger download
  const blob = new Blob([compressed], { type: 'application/gzip' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = file.name + '.gz';
  a.click();
  URL.revokeObjectURL(url);
});