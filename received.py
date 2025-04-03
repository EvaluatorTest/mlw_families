from flask import Flask, request
import os

app = Flask(__name__)
UPLOAD_FOLDER = "uploads"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

@app.route('/upload', methods=['POST'])
def upload():
    filename = request.args.get("name", "unknown.bin")
    with open(os.path.join(UPLOAD_FOLDER, filename), 'ab') as f:
        f.write(request.data)
    return "OK"

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)
