
#ifndef	_REVIEWSESSIONSSCREENS_H
#define	_REVIEWSESSIONSSCREENS_H


#ifdef __cplusplus
extern "C" {
#endif


WM_HWIN GetReviewSessionsWindowHandle(void);
void SetReviewSessionsWindowHandle(WM_HWIN);
extern void ReviewSessionsScreensCallback(WM_MESSAGE *);
void ReviewSessionsIgnoreNextKeyRelease(void);
void DisplaySessionDetailsScreen(void);
void ReturnFromReviewSessionsScreen(void);

#ifdef __cplusplus
}
#endif


#define	NUM_REVIEW_SESSIONS_SCREENS	2

#define	CHECK_TIMES_SCREEN		0
#define	DELETE_SESSION_SCREEN	1


#endif
