/*

  Author: Lubosz Sarnecki
  2013

*/

#ifndef GESDEMO_H
#define GESDEMO_H

#include <unistd.h>
#include <stdio.h>
#include <gst/gst.h>
#include <ges/ges.h>

GESTimeline *testPatternTL (void);
GESTimeline *effectTL (void);
GESTimeline *testTL (void);
GESTimeline *minuteTL (void);
GESTimeline *imageTL (void);
GESTimeline *oneTL (void);
GESTimeline *musicTL (void);
GESTimeline *hdTL (void);
GESTimeline * pngTL (void);
GESTimeline * transitionTL (void);

void playTests(void);
void formatTests(void);
void renderTests(void);
void brokenRenderTests(void);

char *replace(char *s, char old, char replacement);

#endif // GESDEMO_H
