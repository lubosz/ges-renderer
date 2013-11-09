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
GESTimeline *musicTL (void);
GESTimeline *hdTL (void);
GESTimeline *transitionTL (void);
GESTimeline *overlayTL (void);
GESTimeline *videoTransparencyTL (void);
GESTimeline *volumeTestTL (void);
GESTimeline *positionTestTL (void);
GESTimeline *alphaTestTL (void);
GESTimeline *compTL (void);

void playTests (void);
void formatTests (void);
void renderTests (void);
void newTests (void);

#endif // GESDEMO_H
