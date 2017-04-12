var http = require('http');
var url = require('url');
var io = require('socket.io');
var fs = require('fs');
var Gpio = require('onoff').Gpio;
var raspi = require('raspi');
var I2C = require('raspi-i2c').I2C;

var sendstate = 0;
var led1 = new Gpio(17, 'out');
led1.writeSync(0);

raspi.init(function() {
  var i2c = new I2C();
  console.log(i2c.readSync(0x09,4)); // Read one byte from the device at address 18
//  i2c.writeByteSync(0x09,0x19);
});


var server = http.createServer(function(request, response) {
	var path = url.parse(request.url).pathname;

	switch(path) {
		case '/':
			response.writeHead(200, {'Content-Type': 'text/html'});
			response.write('Welcome to Denver Grow');
			response.end();
			break;
		case '/index.html':
			fs.readFile(__dirname + path, function(error, data){
                if (error){
                    response.writeHead(404);
                    response.write("oops this doesn't exist - 404");
                    response.end();
                }
                else{
                    response.writeHead(200, {"Content-Type": "text/html"});
                    response.write(data, "utf8");
                    response.end();
                }
			});
			break;
		default:
			response.writeHead(400);
			response.write("Ain't nuttin' here, chief");
			response.end();
			break;
	}
});


server.listen(8080);
var listener = io.listen(server);

listener.sockets.on('connection', function (socket) {
	console.log("user connected to socket");

	socket.on('controlPin', function(data) {
		console.log("control " + data.pin + ": " + data.state);
		var onPin = new Gpio(parseInt(data.pin), 'out');
		onPin.writeSync(data.state);
	});

	socket.on('i2cByte', function(data) {
		var i2c = new I2C();
		i2c.writeByteSync(data.addr, data.cmd);
		console.log('I2C Byte: ' + data.addr + ": " + data.cmd);
	});

	socket.on('i2cBright', function(data) {
		var i2c = new I2C();
		var cmdSetTL = new Buffer([data.cmd, data.val]);//([0x10, data.val]);
		i2c.writeSync(data.addr, cmdSetTL);
	});

var counter = 0;

	socket.on('datareq', function() {
		// read in I2C bytes to json var datarecd
		var i2c = new I2C();
		var ainbytes = i2c.readSync(0x09,8);
		var afahr = ((ainbytes[0] * 255) + ainbytes[1]) / 100;
		var acel = ((ainbytes[2] * 255) + ainbytes[3]) / 100;
		var aa1tot = (ainbytes[4] * 255) + ainbytes[5];
		var aa2tot = (ainbytes[6] * 255) + ainbytes[7];

		var binbytes = i2c.readSync(0x0a,8);
		var bfahr = ((binbytes[0] * 255) + binbytes[1]) / 100;
		var bcel = ((binbytes[2] * 255) + binbytes[3]) / 100;
		var ba1tot = (binbytes[4] * 255) + binbytes[5];
		var ba2tot = (binbytes[6] * 255) + binbytes[7];
		
		var datarecd = {
			afahr: afahr,
			acel: acel,
			aana1: aa1tot,
			aana2: aa2tot,
			bfahr: bfahr,
			bcel: bcel,
			bana1: ba1tot,
			bana2: ba2tot
		}
		socket.emit('datasent', datarecd); // make this a callback on the read function
	});
	
//	socket.emit('ledstate', sendstate);
});
