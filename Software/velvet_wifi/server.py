from flask import Flask, request, Response, send_file
import io
app = Flask(__name__)

MY_TOKEN = "tarcktor_token_01"

@app.route('/')
def hello_world():
    return 'Hello, World!'


@app.route('/frimware/', methods=['GET','POST'])
def set_camera_cmd():
    if request.method == 'GET':
        token = request.args.get('token')
        if token == MY_TOKEN:
            filepath = "velvet.bin"
            # filepath = "demo.hex"
            with open(filepath, 'rb') as bites:
                response = send_file(
                    io.BytesIO(bites.read()),
                    mimetype="application/octet-stream",
                    as_attachment=True,
                    attachment_filename="velvet.bin"
                )
                return response
        else:
            return Response(response="INCORRECT TOKEN", status=500)
    else:
        return Response(response="WRONG METHOD", status=500)

if __name__ == '__main__':
    host_ip = '172.31.7.161'
    app.run(host=host_ip)
