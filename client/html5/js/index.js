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

class Server {

	constructor(url){
		this.setUrl(url);
		this.socket = null;
		this.onopen = [];
		this.onclose = [];
		this.onmessage = null;
	}

	setUrl(url){
		if(url){
			var absolute_path = /^\w+:\/\/.+/i;
			if(absolute_path.test(url)){
				this.url = url;
			}else{
				this.url = 'ws://' + url;
			}
		}else{
			this.url = '';
		}
	}

	connect(url){
		this.setUrl(url);
		if(!this.url){
			return false;
		}

		var server = this;
		this.socket = new WebSocket(this.url);
		this.socket.onopen = function(){
			for(var i = 0; i < server.onopen.length; i++){
				server.onopen[i]();
			}
		};
		this.socket.onmessage = function (e) {
			var packet = JSON.parse(e.data);
			var command = packet[0];
			var args = packet[1];
			if (server.onmessage && server.onmessage[command]) {
				server.onmessage[command](args);
			}
		};
		this.socket.onclose = function(e) {
			for(var i = 0; i < server.onclose.length; i++){
				server.onclose[i](e);
			}
			server.socket = null;
		};

		return true;
	}

	disconnect(){
		if(this.socket){
			this.socket.close();
			this.socket = null;
		}
	}

	get isConnected(){
		return this.socket != null;
	}

	request(command, args = null){
		if(this.socket){
			var packet = [command, args];
			this.socket.send(JSON.stringify(packet));
			return true;
		}else{
			return false;
		}
	}

	onOpen(callback){
		this.onopen.push(callback);
	}

	onClose(callback){
		this.onclose.push(callback);
	}

	onMessage(command, callback){
		this.onmessage[command] = callback;
	}

};

class Dialog {

	constructor(){
		this.title = '';
		this.titleTag = 'h4';
		this.content = null;
		this.elapse = 3;
	}

	exec(){
		var title = $('<' + this.titleTag + '></' + this.titleTag + '>');
		title.html(this.title);

		var dialog = $('<div></div>');
		dialog.addClass('dialog');
		dialog.append(title);
		if(this.content){
			dialog.append(this.content);
		}

		dialog.hide();
		$('#board').append(dialog);

		dialog.css('top', '+=10px');
		dialog.animate({
			'top' : '-=10px',
			'opacity' : 1
		}, 500);

		if(this.elapse > 0){
			setTimeout(function(){
				dialog.fadeOut(function(){
					dialog.remove();
				});
			}, this.elapse * 1000);
		}
	}

}


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

var server = new Server();
server.onmessage = Callback;

server.onOpen(function(){
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
});

server.onClose(function(e){
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
});

$(function(){
	//Forbid selection
	var body = $('body')[0];
	body.onselectstart = function(){
		return false;
	};

	$('#scene-map').on('click', 'button', function(e){
		var button = $(e.target);
		var id = button.data('id');
		var screen = $('#screen');
		var width = screen.width();
		var height = screen.height();

		if(server.isConnected()){
			server.request(Command.CreateDevice, [width, height, id]);
		}
	});
});
