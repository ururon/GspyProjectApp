var usb = exports = module.exports = require("bindings")("usbWin")
var events = require('events')
var util = require('util')

// Check that libusb was initialized.
if (usb.INIT_ERROR) {
	throw new Error('Could not initialize libusb. Check that your system has a usb controller.');
}

usb.registerHotplug(function(type,vid,pid,usbType){
	console.log('Type:'+type+',vid:'+vid+',pid:'+pid+',usbType:'+usbType);
	test();
});

var test=function(){
	var tt=usb.getDeviceList();
	for (var i = 0; i < tt.length; i++) {
		if(tt[i]!==undefined && tt[i].idVendor===0x0458){
			console.log(tt[i]);
			var dev=new usb.Device(tt[i]);
			console.log(dev.open());

			var buf=new Buffer(0x40);
			buf.fill(0x00);
			console.log(dev.hidSetReport(0x0,0x40,buf));

			console.log(dev.hidGetReport(0x0,0x40));
			console.log(dev.startTransfer('123456',function(sn,data){
				console.log(sn);
				console.log(data);
				//console.log(dev.hidSetReport(0x0,0x40,buf));
				//console.log(dev.hidGetReport(0x0,0x40));
			}));
		}
	};
}

test();