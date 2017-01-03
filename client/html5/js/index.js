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
	SearchPortalKey : 12,

	NumOfCommands : 13
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
				var domain_split = url.indexOf('/');
				var domain = '';
				var path = '';
				if(domain_split == -1){
					domain = url;
				}else{
					domain = url.substr(0, domain_split);
					path = url.substr(domain_split + 1);
				}
				if(domain.indexOf(':') >= 0){
					this.url = 'ws://' + domain + '/' + path;
				}else{
					this.url = 'ws://' + domain + ':52600/' + path;
				}
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
		this.dialog = null;
	}

	exec(elapse){
		if(typeof elapse == 'number'){
			this.elapse = elapse;
		}

		var title = $('<' + this.titleTag + '></' + this.titleTag + '>');
		title.html(this.title);

		var dialog = $('<div></div>');
		dialog.addClass('dialog');
		dialog.addClass('center');
		dialog.append(title);
		if(this.content){
			dialog.append(this.content);
		}

		$('#board').append(dialog);
		dialog.css('top', '+=10px');
		dialog.animate({
			'top' : '-=10px',
			'opacity' : 1
		}, 500);

		this.dialog = dialog;
		if(this.elapse > 0){
			setTimeout(function(){
				this.close();
			}, this.elapse * 1000);
		}
	}

	close(){
		if(this.dialog){
			this.dialog.fadeOut(function(){
				$(this).remove();
			});
			this.dialog = null;
		}
	}

}

class Toast {

	constructor(text){
		this.text = text;
		this.elapse = 3;
		this.onclose = [];
	}

	exec(elapse){
		if(typeof elapse == 'number'){
			this.elapse = elapse;
		}

		var toast = $('<div></div>');
		toast.addClass('toast');
		toast.html(this.text);

		$('#board').append(toast);
		toast.css({opacity: 0, top : '+=10px'});
		toast.animate({opacity : 1, top : '-=10px'}, 500);

		var object = this;
		setTimeout(function(){
			toast.remove();
			for(var i = 0; i < object.onclose.length; i++){
				object.onclose[i]();
			}
		}, this.elapse * 1000);
	}

	onClose(callback){
		this.onclose.push(callback);
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
	var dialog = new Dialog();
	dialog.title = 'Please select a scene';

	var ul = $('<ul></ul>');
	ul.addClass('scene-map');
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

	ul.on('click', 'button', function(e){
		var button = $(e.target);
		var id = button.data('id');
		var screen = $('#screen');
		var width = screen.width();
		var height = screen.height();

		if(server.isConnected){
			server.request(Command.CreateDevice, [width, height, id]);
			var search_dialog = $('#search-dialog');
			search_dialog.css('visibility', 'visible');
		}

		dialog.close();
	});

	dialog.content = ul;
	dialog.exec(0);
};

var server = new Server();
server.onmessage = Callback;

server.onOpen(function(){
	$('#connect-dialog').hide();

	var toast = new Toast('Connection established.');
	toast.onClose(function(){
		server.request(Command.ListMap);
	});
	toast.exec(1);

	var screen = $('#board');

	screen.mousedown(function(e){
		if(!$(e.target).is('#board')){
			return;
		}
		e.preventDefault();
		Camera.attached = true;
		Camera.x = e.pageX;
		Camera.y = e.pageY;
	});

	screen.mouseup(function(){
		Camera.attached = false;
	});

	screen.mousemove(function(e){
		if(!Camera.attached){
			return;
		}

		var deltaX = e.pageX - Camera.x;
		var deltaY = e.pageY - Camera.y;
		Camera.x = e.pageX;
		Camera.y = e.pageY;
		server.request(Command.RotateCamera, [deltaX * 100, deltaY * 100]);
	});

	screen.mousewheel(function(e){
		e.preventDefault();
		server.request(Command.ScaleCamera, [-e.deltaY * 2000]);
	});
});

server.onClose(function(e){
	var toast = new Toast;
	switch(e.code){
	case 1000:
		toast.text = 'Server is closed.';
		break;
	case 1006:
		toast.text = 'Failed to establish a connection.';
		break;
	default:
		toast.text = 'Unknown error.';
	}

	toast.onClose(function(){
		$('#connect-dialog').show();
	});

	toast.exec();
});

$(function(){
	//Forbid selection
	var body = $('body')[0];
	body.onselectstart = function(){
		return false;
	};

	$('#connect-form').submit(function(e){
		e.preventDefault();

		var message_box = $(e.target).find('.message');
		message_box.html('Connecting...');

		var server_url = e.target.server_url.value;
		server.connect(server_url);
	});

	$('#search-form').submit(function(e){
		e.preventDefault();
		var input = e.target.keyword;
		var keyword = input.value;
		keyword.replace(/^[\s\r\n]+/g, '');
		keyword.replace(/[\s\r\n]+$/g, '');
		server.request(Command.SearchPortalKey, [keyword]);
		input.value = '';
	});
});
