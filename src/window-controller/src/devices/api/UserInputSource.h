#ifndef USER_INPUT_SOURCE_H
#define USER_INPUT_SOURCE_H

class UserInputSource {
public:
    virtual ~UserInputSource() {}

    virtual void setup() = 0;
    virtual bool isModeButtonPressed() = 0; // Evento di pressione (debounced)
    virtual int getPotentiometerPercentage() = 0; // Lettura diretta 0-100
    // Potremmo aggiungere una versione "if_changed" se la logica FSM lo richiede esplicitamente
    // virtual int getPotentiometerPercentageIfChanged(int& lastReadPercentage) = 0;
};

#endif // USER_INPUT_SOURCE_H