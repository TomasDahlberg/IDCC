  } else if (typ == 15) {
    struct _calendar *cal;
    cal = (struct _calendar *) metaValue(dm, meta, id);
    printf("calendar %s;\n", (char *) metaName(meta, id));
    for (i = 0; i < 10; i++) {
      fgets(line, 80, fpLog);
      parseCalendar(line, cal, i);
    }



void parseCalendar(char *line, struct _calendar *cal, int i)
{
  int w, weekDay = 0, color, stopDay = 0, startTime, stopTime;
  char day[] = { 'M', 'T', 'W', 'T', 'F', 'S', 'S' };

  for (w = 0; w < 7; w++) 
    if (line[w*3] == day[w])
	weekDay |= (1 << w);
  if (weekDay == 0) {
    weekDay = atoi(&line[0]);
    stopDay = atoi(&line[7]);
  } else
    weekDay |= 2048;

  if (line[23] == 'B') color = 1;
  else if (line[23] == 'O') color = 2;
  else if (line[23] == 'R') color = 4;
  else color = 0;

  if (line[32] == 'H') {
    startTime = 0;     
    stopTime = 2400;
  } else {
    double atof(), d;
    d = 100.0*atof(&line[32]);
    startTime = d;
    d = 100.0*atof(&line[40]);
    stopTime = d;
  }
  cal->day[i] = (hilo) ? weekDay : swapword(weekDay);
  cal->stopday[i] = (hilo) ? stopDay : swapword(stopDay);
  cal->color[i] = color;
  cal->start[i] = (hilo) ? startTime : swapword(startTime);
  cal->stop[i] = (hilo) ? stopTime : swapword(stopTime);
}

