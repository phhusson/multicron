namespace UEvents {
	class Power : public  Event {
		public:
			Power(UEvents::Event *ev);
			Power();
			~Power();
			void SetVar(const char *name, const char *value);
			void Display();
		private:
			//Informations comes from linux 2.6.30-rc6 drivers/power/power_supply_sysfs.c
			char *name;//POWER_supply_name

			enum power_type {
				BATTERY,
				UPS,
				MAINS,
				USB
			} type;//POWER_SUPPLY_TYPE

			enum power_status {
				UNKNOWN_STATUS,
				CHARGING,
				DISCHARGING,
				NOTCHARGING,
				FULL
			} status;//POWER_SUPPLY_STATUS

			bool present;//POWER_SUPPLY_PRESENT

			enum power_technology {
				UNKNOWN_TECHNOLOGY,
				NIMH,
				LIION,
				LIPOLY,
				LIFE,
				NICD,
				LIMN
			} technology;

			int voltage_min_design;
			int voltage_now;
			int current_now;
			int charge_full_design;
			int charge_full;
			int charge_now;
			char *model_name;
			char *manufacturer;
			char *serialnum;

	};
};
