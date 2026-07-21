#include <Arduino.h>

#include "Display.h"
#include "InputManager.h"
#include "UIManager.h"

namespace
{
  void stopWithError(const char *message)
  {
    Serial.println(message);

    while (true)
    {
      delay(1000);
    }
  }

  const char *actionToText(
      const InputManager::Action action)
  {
    switch (action)
    {
    case InputManager::Action::Previous:
      return "PREVIOUS";

    case InputManager::Action::Next:
      return "NEXT";

    case InputManager::Action::Select:
      return "SELECT";

    case InputManager::Action::Back:
      return "BACK";

    case InputManager::Action::Home:
      return "HOME";

    case InputManager::Action::None:
    default:
      return "NONE";
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("Booting VMT OS...");

  if (!Display::begin())
  {
    stopWithError(
        "ERROR: Display initialization failed.");
  }

  if (!InputManager::begin())
  {
    stopWithError(
        "ERROR: Input initialization failed.");
  }

  if (!UIManager::begin())
  {
    stopWithError(
        "ERROR: UI initialization failed.");
  }

  Serial.println("System ready.");
}

void loop()
{
  const InputManager::Action action =
      InputManager::update();

  if (action != InputManager::Action::None)
  {
    Serial.print("Input: ");
    Serial.println(actionToText(action));

    UIManager::handleInput(action);
  }

  UIManager::update();
}
