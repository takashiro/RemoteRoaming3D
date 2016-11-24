var Command = {
	Invalid : 0,

	UpdateVideoFrame : 1,
	MakeToastText : 2,
	EnterHotspot : 3,
	Quit : 4,
	CreateDevice : 5,
	RotateCamera : 6,
	ScaleCamera : 7,
	MoveCamera : 8,
	ControlHotspots : 9,
	DoubleClick : 10,
	ListMap : 11,

	NumOfCommands : 12
};

var Camera = {
	attached: false,
	x: 0,
	y: 0
};

var Callback = new Array(Command.NumOfCommands);

Callback[Command.UpdateVideoFrame] = function(arg) {
	var screen = $('#screen');
	var img = screen.children('img');
	if (img.length > 0){
		img.attr('src', 'data:image/jpeg;base64,' + arg);
	} else {
		screen.html('<img src="data:image/jpeg;base64,' + arg + '">');
	}
};

Callback[Command.ListMap] = function(args) {
	$('#screen').html('请选择一个场景');

	var ul = $('#scene-map');
	ul.html('');
	for(var i = 0; i < args.length; i++) {
		var info = args[i];
		var button = $('<button></button>');
		button.html(info.name);
		button.data('id', info.id);
		button.attr('title', info.description);
		var li = $('<li></li>');
		li.append(button);
		ul.append(li);
	}
};

var Server = {
	url : 'ws://127.0.0.1:52600/',
	socket : null,

	connect : function () {
		this.socket = new WebSocket(this.url);
		this.socket.onmessage = function (e) {
			var packet = JSON.parse(e.data);
			var command = packet[0];
			var args = packet[1];
			if (Callback[command]) {
				Callback[command](args);
			}
		};
	},

	request : function (command, args = null) {
		var packet = [command, args];
		this.socket.send(JSON.stringify(packet));
	}
};

$(function(){
	var body = $('body')[0];
	body.onselectstart = function(){
		return false;
	};

	$('#screen').html('正在连接服务器……');
	Server.connect();

	Server.socket.onopen = function(){
		$('#screen').html('连接成功');
		Server.request(Command.ListMap);

		var screen = $('#screen');

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
	};

	Server.socket.onclose = function(e){
		switch(e.code){
		case 1000:
			$('#screen').html('服务器已关闭');
			break;
		case 1006:
			$('#screen').html('无法连接到服务器');
			break;
		default:
			$('#screen').html('未知错误');
		}
	};

	$('#scene-map').on('click', 'button', function(e){
		var button = $(e.target);
		var id = button.data('id');
		var screen = $('#screen');
		var width = screen.width();
		var height = screen.height();

		screen.html('加载中……');
		Server.request(Command.CreateDevice, [width, height, id]);
	});
});
