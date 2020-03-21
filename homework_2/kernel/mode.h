// DOMACI
struct {
	unsigned int o : 1; // 0 = closed, 1 = open
	unsigned int c : 1; // 0 = explorer, 1 = clipboard
	unsigned int a : 1; // 0 = f1 deactivated, 1 = f1 activated
	unsigned int e : 1; // 0 = input disabled, 1 = input enabled
	unsigned int x : 1; // 0 = no exception, 1 = exception
	unsigned int d : 1; // 0 = clipboard clean, 1 = clipboard dirty
} mode;
