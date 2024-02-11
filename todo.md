
TODO:
- key press processed no matter which window is in focus
    - key press sent to game window

classes in txt files
press key to start the rotation specified in the file
press key to stop and start from beginning when continuing
file has:
    - class name
    - skills
        - cooldowns
    - rotation


  /**
  regex_t regexSkill;
  int retiS = regcomp(&regexSkill, "^\d+: [\d.]+$", 0);

  regex_t regexCombo;
  int retiC = regcomp(&regexCombo, "^[a-pr-v-zA-PR-V-Z] \d+(>\d+)*$", 0);

  if (retiC || retiS) {
    fprintf(stderr, "Couldn't compile regex\n");
    exit(1);
  }
  **/
