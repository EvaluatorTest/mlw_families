from flask import Flask, request
import os

app = Flask(__name__)
SAVE_DIR = "uploaded_music"
os.makedirs(SAVE_DIR, exist_ok=True)

@app.route('/upload', methods=['POST'])
def receive_file():
    filename = request.args.get("name", "unnamed_file")
    path = os.path.join(SAVE_DIR, filename)
    with open(path, "ab") as f:
        f.write(request.data)
    return "File received."

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=8080)
