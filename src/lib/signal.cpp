#include "guiding_light/signal.hpp"

#include <csignal>
#include "guiding_light/logging.hpp"

void handle_signal(const int code) {
	fatality f;
	switch (code) {
	case SIGINT:
		log("Ну, все, я пішов спати, бувай, добраніч.");
		std::cout << "Ну, все, я пішов спати, бувай, добраніч.\n";
		// Ukrainian for "Well, that's it, I'm going to bed, bye, goodnight."
		f = f_non_fatal;
		break;
	case SIGTERM:
		log("I have been told to kill myself, doing it now.");
		std::cout << "I have been told to kill myself, doing it now.\n";
		f = f_non_fatal;
		break;
	case SIGSEGV:
		log("I am not entitled to this memory.");
		std::cout << "I am not entitled to this memory.\n";
		f = f_fatal;
		break;
	default:
		log(fmt::format("Some other signal idk {}", code));
		std::cout << fmt::format("Some other signal idk {}", code);
		f = f_fatal;
		break;
	}
	if (!IS_CLI) {
		dump_data(f);
	}
}
