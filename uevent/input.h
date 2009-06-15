namespace UEvents {
	class Input : public  Event {
		public:
			Input(UEvents::Event *ev);
			bool Match(cfgNode);
			Input();
			~Input();
			void SetVar(const char *name, const char *value);
			void Display();
			void FillCtx(struct context &ctx);
		private:
			void Zero();

			char *product;
			char *name;
			char *phys;

			//Some others to add
	};
};
