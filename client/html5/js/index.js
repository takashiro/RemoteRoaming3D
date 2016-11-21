var Command = {
	Invalid : 0,

	//Server to Client
	UpdateVideoFrame : 1,
	MakeToastText : 2,
	EnterHotspot : 3,
	Quit : 4,

	//Client to Server
	CreateDevice : 5,

	RotateCamera : 6,
	ScaleCamera : 7,
	MoveCamera : 8,

	ControlHotspots : 9,
	DoubleClick : 10,

	NumOfCommands : 11
};

var Camera = {
	attached: false,
	x: 0,
	y: 0
};

var Server = {
	url : 'ws://127.0.0.1:52600/',
	socket : null,

	connect : function () {
		var screen = $('#screen');
		screen.html('Loading...');

		this.socket = new WebSocket(this.url);
		this.socket.onmessage = function (e) {
			var packet = JSON.parse(e.data);
			if (packet[0] == Command.UpdateVideoFrame) {
				var img = screen.children('img');
				if (img.length > 0){
					img.attr('src', 'data:image/jpeg;base64,' + packet[1]);
				} else {
					screen.html('<img src="data:image/jpeg;base64,' + packet[1] + '">');
				}
			}
		};
	},

	request : function (command, args) {
		var packet = [command, args];
		this.socket.send(JSON.stringify(packet));
	}
};

$(function(){
	var body = $('body')[0];
	body.onselectstart = function(){
		return false;
	};

	Server.connect();

	var screen = $('#screen');
	var width = screen.width();
	var height = screen.height();
	Server.socket.onopen = function(){
		Server.request(Command.CreateDevice, [width, height]);
	};

	screen.mousedown(function(e){
		e.preventDefault();
		Camera.attached = true;
		Camera.x = e.pageX;
		Camera.y = e.pageY;
	});

	screen.mouseup(function(){
		Camera.attached = false;
	});

	screen.mousemove(function(e){
		if (Camera.attached) {
			var deltaX = e.pageX - Camera.x;
			var deltaY = e.pageY - Camera.y;
			Camera.x = e.pageX;
			Camera.y = e.pageY;
			Server.request(Command.RotateCamera, [deltaX * 100, deltaY * 100]);
		}
	});

	screen.mousewheel(function(e){
		e.preventDefault();
		Server.request(Command.ScaleCamera, [-e.deltaY * 2000]);
	});
});
