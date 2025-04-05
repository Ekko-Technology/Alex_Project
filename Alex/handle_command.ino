void handleCommand(TPacket *command)
{
  int speed = 75;
  switch(command->command)
  {
    // For movement commands, param[0] = distance, param[1] = speed.
    case COMMAND_FORWARD:
      sendOK();
      fforward((double) 4, (float) speed);
      break;

    case COMMAND_REVERSE:
      sendOK();
      reverse((double) 4, (float) speed);
      break;
    case COMMAND_LONGFORWARD:
      sendOK();
      fforward((double) 12, (float) speed);
      break;

    case COMMAND_LONGREVERSE:
      sendOK();
      reverse((double) 12, (float) speed);
      break;

    case COMMAND_TURN_LEFT:
      sendOK();
      left((double) command->params[0], (float) speed);
      break;

    case COMMAND_TURN_RIGHT:
      sendOK();
      right((double) command->params[0], (float) speed);
      break;

    case COMMAND_STOP:
      sendOK();
      stop();
      break;

    case COMMAND_GET_STATS:
      sendOK();
      sendStatus();
      break;

    case COMMAND_CLEAR_STATS:
      sendOK();
      clearOneCounter(command->params[0]);
      break;

    case COMMAND_TOGGLECLAW:
      sendOK();
      toggle_claw();
      sendMessage("Toggling arm");
      break;
    
    case COMMAND_DETECTCOLOUR:
      sendOK();
      sendMessage("Detecting Colour");
      detectColour();
      break;
    case COMMAND_ULTRASONIC:
      sendOK();
      detectDistance();
      sendMessage("Detecting Distance");
      break;
    default:
      sendBadCommand();
  }
}
