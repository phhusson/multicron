namespace UEvents {
	class USB : public  Event {
		public:
			USB(UEvents::Event *ev);
			bool Match(xmlNode);
			USB();
			~USB();
			void SetVar(const char *name, const char *value);
			void Display();
		private:
			void Zero();
			//Informations comes from linux 2.6.30-rc6 drivers/usb/core/{driver.c,message.c,usb.c}
			
			//Unparsed infos
			char *device; // = /proc/bus/usb/$(busnum)/$(devnum)
			char *product; // = idVendor/idProduct/bcdDevice
			char *type;//bDeviceClass/bDeviceSubClass/bDeviceProtocol
			char *interface;// = bInterfaceClass/bInterfaceSubClass/bInterfaceProtocol
			short busnum;//defined as %03d
			short devnum;//same
			char *modalias;//just a summary of previous infos

			//?!
			char *devtype;

			//Parsed ones
			int bInterfaceClass;
			int bInterfaceSubClass;
			int bInterfaceProtocol;
			int idVendor;
			int idProduct;
			int bcdDevice;
			int bDeviceClass;
			int bDeviceSubClass;
			int bDeviceProtocol;

	};
};
