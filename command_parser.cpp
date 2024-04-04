


char* GET() {
	return "Unimplemented";
}

char* SET() {
	return "Unimplemented";
}

char* DELETE() {
	return "Unimplemented";
}






// Main driver function, identify if valid command then send away to functions to execute command.
// Return either confirmation of success or error
char* unpack_command(char* commandString, int length) {

	// Test command to make sure it matches a supported command

	const char* Commands[] = { "GET", "SET", "DELETE"};
	
	int commandCandidates = sizeof(Commands)/sizeof(Commands[0]);
	int commandArrSize = commandCandidates;
	int lastValidCommand = 0;
	int step = 0;

	// parse out command that is at the beginning of the string if there is one
	while (scanner != ' ' || scanner != '\0' || scanner != '\n') {
		
		int validCommands = 0;
		char c = commandString[step]
		
		for (int i = 0; i < commandArrSize; i++) {
			if (Commands[i][step] == c) {
				lastValidCommand = i;
				validCommands++;
			}
		}

		
		
		step++;
	}

	if (commandCandidates != 1) {
		return "CommandError: server - command not recognised";
	}

	// Check one valid command left
	// Select function based on valid command 

	int command = 0;
	switch (command) {
		case 0:
			GET();
			break;

		case 1:
			SET();
			break;

		case 2:
			DELETE();
			break
		default:
			return "Bad command";


	}






}