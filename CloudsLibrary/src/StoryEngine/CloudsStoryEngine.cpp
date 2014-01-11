//
//  CloudsStoryEngine.cpp
//  CloudsFCPParser
//
//  Created by James George on 3/17/13.
//
//

#include "CloudsStoryEngine.h"
#include "CloudsAct.h"

bool logsort(pair<float,string> a, pair<float,string> b ){
    return a.first > b.first;
}

CloudsStoryEngine::CloudsStoryEngine(){
    parser = NULL;
    visualSystems = NULL;
    customAct = NULL;
	
    isSetup = false;
    printDecisions = true;
    combinedClipsOnly = false;
    printCriticalDecisions = false;
    
    maxTimesOnTopic = 4;
    voClipGapTime = 1.0;
	clipGapTime = .4;
	
//    systemMaxRunTime = 60*2;
    maxVisualSystemGapTime = 60*3;
    longClipThreshold = 30;
    longClipFadeInPercent = .5;
    actLength = 10 * 60;
    cadenceForTopicChangeMultiplier =  10;
    
    lastClipSharesTopicBoost = 10;
    twoClipsAgoSharesTopicBoost = 10;
    
    topicRelevancyMultiplier = 100;
    topicsInCommonMultiplier = 10;
    topicsinCommonWithPreviousMultiplier = 5;
    samePersonOccurrenceSuppressionFactor = 4;
    dichotomyWeight = 2;
    linkFactor = 20;
    maxTimeWithoutQuestion = 120;
    gapLengthMultiplier = 0.5;
    preRollDuration = 5;
    minClipDurationForStartingOffset = 30;

	//this will stop the story from digressing during the first few clips on topic
    minTimesOnTopic = 0;
    distantClipSuppressionFactor = 0;
    
    genderBalanceFactor = 5;
    goldClipFactor = 50;
    easyClipScoreFactor = 60;
    
    visualSystemPrimaryTopicBoost = 10;
    visualSystemSecondaryTopicBoost = 5;
	
	bCreateLog = true;
	bLogTopicDetails = true;
	bLogClipDetails = false;
	bLogVisualSystemDetails = true;

}

CloudsStoryEngine::~CloudsStoryEngine(){
    delete gui;
    delete clipGui;
}

void CloudsStoryEngine::setup(){
    if(!isSetup){
        isSetup = true;
        
        initGui();
        
        dichotomyThreshold = 3;

		dichotomies = CloudsDichotomy::getDichotomies();
    }
}

void CloudsStoryEngine::initGui(){
    
    runGui = new ofxUISuperCanvas("RUN INFORMATION",OFX_UI_FONT_SMALL);
    runGui->addSpacer();
    runGui->setWidgetFontSize(OFX_UI_FONT_SMALL);
    
    actGui = new ofxUISuperCanvas("ACT PARAMS", OFX_UI_FONT_SMALL);
    
    vector<CloudsClip> startingNodes = parser->getClipsWithKeyword("#start");
    vector<string> questions;
    for(int i = 0; i < startingNodes.size(); i++){
        CloudsClip& clip = startingNodes[i];
        if(!clip.hasQuestion()){
            questions.push_back("#START CLIP " + ofToUpper(clip.getLinkName()) + " HAS NO QUESTION **");
        }
        else{
            map<string,string> clipQuestions = clip.getAllQuestionTopicPairs();
            questions.push_back( clip.getLinkName() + ", " + clipQuestions.begin()->first + ", " + clipQuestions.begin()->second );
        }
    }
    
    actGui->setWidgetFontSize(OFX_UI_FONT_SMALL);
    ofxUIDropDownList* ddQuestions = actGui->addDropDownList("STARTING QUESTIONS", questions, 700);
    ddQuestions->setAutoClose(true);
    ddQuestions->setShowCurrentSelected(true);
    ddQuestions->setAllowMultiple(false);
    actGui->autoSizeToFitWidgets();
    
    clipGui = new ofxUISuperCanvas("CLIP SCORE PARAMS", OFX_UI_FONT_SMALL);
    clipGui->setPosition(0,0);
    clipGui->addSpacer();
	
    clipGui->addSlider("TOPICS IN COMMON CURRENT MULTIPLIER", 0, 50, &topicsInCommonMultiplier);
    clipGui->addSlider("TOPICS IN COMMON PREVIOUS MULTIPLIER", 0, 10, &topicsinCommonWithPreviousMultiplier);
    clipGui->addSlider("SAME PERSON SUPPRESSION FACTOR", 0, 10, &samePersonOccurrenceSuppressionFactor);
    clipGui->addSlider("LINK FACTOR",0,50, &linkFactor);
    clipGui->addSlider("DICHOTOMY WEIGHT", 0,100, &dichotomyWeight);
    clipGui->addSlider("GENDER BALANCE", 0, 10, &genderBalanceFactor);
    clipGui->addSlider("DISTANT CLIP SUPRRESSION", 0, 100, &distantClipSuppressionFactor);
    clipGui->addSlider("GOLD CLIP FACTOR", 10, 100, &goldClipFactor);
    clipGui->addSlider("EASY CLIP FACTOR", 10, 100, &easyClipScoreFactor);
    
    clipGui->autoSizeToFitWidgets();
    
    topicGui = new ofxUISuperCanvas("TOPIC SCORE PARAMS", OFX_UI_FONT_SMALL);
    topicGui->addSpacer();
    topicGui->addSlider("RELEVANCY MULTIPLIER", 0, 200, &topicRelevancyMultiplier);
    topicGui->addSlider("LAST CLIP SHARES TOPIC BOOST", 0, 20, &lastClipSharesTopicBoost);
    topicGui->addSlider("TWO CLIPS AGO SHARES TOPIC BOOST",0, 20, &twoClipsAgoSharesTopicBoost);
    topicGui->autoSizeToFitWidgets();
    
    gui = new ofxUISuperCanvas("TIMING PARAMTERS", OFX_UI_FONT_SMALL);
    gui->setPosition(clipGui->getRect()->x + 100, clipGui->getRect()->y + 10);
	
    gui->addSpacer();
	gui->addIntSlider("MAX TOPICS PER ACT", 2, 5, &maxTopicsPerAct);
	gui->addIntSlider("MIN TIMES ON TOPIC", 0, 7, &minTimesOnTopic);
    gui->addIntSlider("MAX TIMES ON TOPIC", 0, 7, &maxTimesOnTopic);
	gui->addSpacer();
	gui->addSlider("CLIP GAP", .2, 1.0, &clipGapTime);
	gui->addSlider("VO CLIP GAP", .2, 10.0, &voClipGapTime);
    gui->addSlider("PREROLL FLAG TIME", 1, 10, &preRollDuration);
	gui->addSpacer();
	gui->addSlider("MAX VS RUNTIME", 0, 60, &maxVisualSystemRunTime);
    gui->addSlider("MAX VS GAPTIME", 0, 60, &maxVisualSystemGapTime);
	gui->addSlider("TOPIC END VISUAL EXTEND", 3.0, 30., &visualSystemTopicEndExtend);
	//    gui->addSlider("GAP LENGTH MULTIPLIER", 0.01, 0.1, &gapLengthMultiplier);
	//    gui->addSlider("ACT LENGTH", 60, 1200,&actLength);
	//    gui->addSlider("MIN CLIP DURATION FOR START OFFSET", 0, 100, &minClipDurationForStartingOffset);
    gui->autoSizeToFitWidgets();
    
    vsGui = new ofxUISuperCanvas("VISUAL SCORE PARAMS", OFX_UI_FONT_SMALL);
    vsGui->setPosition(gui->getRect()->x + 100, gui->getRect()->y + 10);
    vsGui->addSpacer();
    vsGui->addSlider("PRIMARY TOPIC BOOST", 0, 20, &visualSystemPrimaryTopicBoost);
    vsGui->addSlider("SECONDARY TOPIC BOOST", 0, 20, &visualSystemSecondaryTopicBoost);
//    vsGui->addSlider("LONG CLIP THRESHOLD", 0, 100,&longClipThreshold);
//    vsGui->addSlider("LONG CLIP FAD IN %", 0.0, 1.0, &longClipFadeInPercent);
//    vsGui->addSlider("CADENCE FOR TOPIC CHANGE", 1, 30, &cadenceForTopicChangeMultiplier);
    vsGui->autoSizeToFitWidgets();
    
	
	logGui = new ofxUISuperCanvas("LOG PARAMS", OFX_UI_FONT_SMALL);
    logGui->setPosition(gui->getRect()->x + 100, gui->getRect()->y + 10);
    logGui->addSpacer();
	logGui->addToggle("CREATE LOG", &bCreateLog);
	logGui->addToggle("LOG TOPICS", &bLogTopicDetails);
	logGui->addToggle("LOG CLIPS", &bLogClipDetails);
	logGui->addToggle("LOG VISUALS", &bLogVisualSystemDetails);

	logGui->autoSizeToFitWidgets();

    string filePath;
    filePath = GetCloudsDataPath() +"storyEngineParameters/gui.xml";
    if(ofFile::doesFileExist(filePath))gui->loadSettings(filePath);
    
    filePath = GetCloudsDataPath() +"storyEngineParameters/clipGui.xml";
    if(ofFile::doesFileExist(filePath))clipGui->loadSettings(filePath);
    
    filePath = GetCloudsDataPath() +"storyEngineParameters/vsGui.xml";
    if(ofFile::doesFileExist(filePath))vsGui->loadSettings(filePath);
    
    filePath = GetCloudsDataPath() +"storyEngineParameters/topicGui.xml";
    if(ofFile::doesFileExist(filePath))topicGui->loadSettings(filePath);
    
    ofAddListener(actGui->newGUIEvent, this, &CloudsStoryEngine::guiEvent);
    ofAddListener(gui->newGUIEvent, this, &CloudsStoryEngine::guiEvent);
    ofAddListener(clipGui->newGUIEvent, this, &CloudsStoryEngine::guiEvent);
    ofAddListener(vsGui->newGUIEvent, this, &CloudsStoryEngine::guiEvent);
    ofAddListener(topicGui->newGUIEvent, this, &CloudsStoryEngine::guiEvent);
    
    positionGuis();
    toggleGuis();    
}

void CloudsStoryEngine::positionGuis(){
    clipGui->setPosition(0,0);
    topicGui->setPosition(clipGui->getRect()->getMaxX(), clipGui->getRect()->y);
    gui->setPosition(topicGui->getRect()->getMaxX(), topicGui->getRect()->y);
    vsGui->setPosition(gui->getRect()->getMaxX(), gui->getRect()->y);
    actGui->setPosition(0, clipGui->getRect()->getMaxY());
	logGui->setPosition(vsGui->getRect()->getMaxX(), vsGui->getRect()->y);
    runGui->setPosition(logGui->getRect()->getMaxX(),0);
}

void CloudsStoryEngine::guiEvent(ofxUIEventArgs &e)
{
    string name = e.widget->getName();
    if(name == "STARTING QUESTIONS"){
        ofxUIDropDownList* b = (ofxUIDropDownList*) e.widget;
        if(! b->isOpen() && b->getSelectedIndeces().size() > 0){
            
            CloudsClip clip = parser->getClipsWithKeyword("#start")[ b->getSelectedIndeces()[0] ];
            
            if(clip.getAllQuestionTopicPairs().size() > 0){
                cout << "Selected index  is " << b->getSelectedIndeces()[0] << endl;                
                string topic = clip.getAllQuestionTopicPairs().begin()->first;
                cout << "SELECTED CLIP ** " << clip.getLinkName() << " WITH TOPIC " << topic << endl;
            
                buildAct(runTest, clip, topic);
                updateRunData();
            }
            else{
                ofLogError("CloudsStoryEngine::guiEvent") << "Selected index  is " << b->getSelectedIndeces()[0] << " has no questions" << endl;
            }
        }
    }
}

void CloudsStoryEngine::updateRunData(){
    
    if(runTest.timesOnCurrentTopicHistory.size()>0){
        runGui->removeWidgets();
        
        map<string, int>::iterator it;
        runTopicCount.clear();
        for(it =runTest.timesOnCurrentTopicHistory.begin(); it != runTest.timesOnCurrentTopicHistory.end(); it++){
            string topicString = it->first + " : " + ofToString(it->second);
            runTopicCount.push_back(topicString);
        }
        runGui->setWidgetFontSize(OFX_UI_FONT_SMALL);
        ofxUIDropDownList* topicCount   = runGui->addDropDownList("RUN TOPIC COUNT", runTopicCount);
        
        topicCount->setAutoClose(true);
        topicCount->setShowCurrentSelected(true);
        topicCount->setAllowMultiple(false);

    }
}

void CloudsStoryEngine::saveGuiSettings(){
    gui->saveSettings(GetCloudsDataPath() +"storyEngineParameters/gui.xml");
    clipGui->saveSettings(GetCloudsDataPath() +"storyEngineParameters/clipGui.xml");
    vsGui->saveSettings(GetCloudsDataPath() +"storyEngineParameters/vsGui.xml");
    topicGui->saveSettings(GetCloudsDataPath() +"storyEngineParameters/topicGui.xml");
    runGui->saveSettings(GetCloudsDataPath() +"storyEngineParameters/runGui.xml");
}

void CloudsStoryEngine::toggleGuis(bool actOnly){
    actGui->toggleVisible();
    if(!actOnly){
        clipGui->toggleVisible();
        topicGui->toggleVisible();
        gui->toggleVisible();
        vsGui->toggleVisible();
        runGui->toggleVisible();
		logGui->toggleVisible();
    }
}

void CloudsStoryEngine::setCustomAct(CloudsAct* act){
	customAct = act;
}

#pragma mark INIT ACT
CloudsAct* CloudsStoryEngine::buildAct(CloudsRun& run, CloudsClip& seed){
	if(seed.getKeywords().size() == 0){
		ofLogError("CloudsStoryEngine::buildAct") << seed.getLinkName() << " contains no keywords!";
		return NULL;
	}
    return buildAct(run, seed, seed.getKeywords()[ ofRandom(seed.getKeywords().size()) ]);
}

CloudsAct* CloudsStoryEngine::buildAct(CloudsRun& run, CloudsClip& seed, string seedTopic, bool playSeed){
	
	//this hack let's us inject custom apps 
	if(customAct != NULL){
		CloudsActEventArgs args(customAct);
		ofNotifyEvent(events.actCreated, args);
		customAct = NULL;
		return NULL;
	}
	
    //the run now listens to act events and is updated through them.
    //making a local copy of the current run to build the new act.
//    vector<CloudsClip> localClipHistory = run.clipHistory;
//    vector<string> localPresetHistory = run.presetHistory;
//    vector<string> localTopicHistory = run.topicHistory;
	///????
//    map<string, int> localTimesOnCurrentTopicHistory = run.timesOnCurrentTopicHistory;
    
	
	//MIN VS WAIT TIME
	//MIN/MAX VS TIME
	//MIN/MAX INTERSTITCHAL TIME
	//first sequence a VS & sound preset for the intro
	//MIN MAX CLIPS PER TOPIC
	
	//# TOPICS PER ACT

	
	//begin laying down clips based on seed topic
	
	//when we encounter MIN VS WAIT TIME
	//begin asking clips if they have good VS
	//	if a clip has a VS lay down (consider audio & consider when in the clip)
	//	if a clip vo only allow it if it has a good vs || a vs is already on
	//  if a VS is running, increase gap lengths and lay down clips
	//	if the topic can change (between MIN & MAX clips) find an interstitchal
	//	allow VO to follow interstichal VS
	//  if topic is last topic, revert to original topic -- promote conclusions + gold (last clip?)

	//clear variables
	clearDichotomiesBalance();

	CloudsStoryState state;
    state.act = new CloudsAct();
	state.act->defaulPrerollDuration = preRollDuration;
    state.clip = seed;
	state.topic = seedTopic;
	state.clipHistory = run.clipHistory;
    state.presetHistory = run.presetHistory;
    state.topicHistory = run.topicHistory;
	state.moreMenThanWomen = 0;
	state.run = run.actCount;
	state.topicNum = 1;
	state.visualSystemRunning = false;
    state.visualSystemEndTime = 0;
	state.duration = 0;
	state.timesOnCurrentTopic = 0;
	state.freeTopic = false;

//    state.run = 2; //RIGGED TO ACT TWO

	state.log << "SEED TOPIC:  " << seedTopic << endl << "SEED CLIP: " << seed.getLinkName() << endl;
	//PLAY FIRST CLIP
	if(playSeed){
		state.log << "\tPlaying seed" << endl;
		
		state.duration = state.act->addClip(state.clip,
											 state.topic,
											 state.duration,
											 dichotomies);
		
		state.clipHistory.push_back(state.clip);
		state.timesOnCurrentTopic++;
	}
	
	state.topicHistory.push_back(seedTopic);
	
	//FIND MATCHING VISUAL SYSTEM
  	state.preset = selectVisualSystem(state, false);
	state.visualSystemStartTime = 0;
	
	if(!state.preset.randomlySelected){
		state.log << "First visual system preset is selected : " <<
					state.preset.getID() << " for topic : " <<
					seedTopic << " and clip " <<
					state.clip.getLinkName() << endl;
		
		state.presetHistory.push_back(state.preset.getID());
		
		if (!state.preset.indefinite) {
			//definite time that this preset must end
			//TODO: CONFIRM PRESET IS NOT TOO SHORT FOR VO CLIP!
			if(state.preset.duration < state.clip.getDuration()){
				state.log << "ERROR: Definite Preset " << state.preset.getID() << " too short for clip " << state.clip.getID() << endl;
			}
			state.visualSystemEndTime = state.preset.duration;
		}
		else{
			state.visualSystemEndTime = maxVisualSystemRunTime;
		}
		state.visualSystemRunning = true;
		
	}
	else {
		state.log << "ERROR: No preset found for intro clip " << state.clip.getLinkName() << endl;
		ofLogError("CloudsStoryEngine::buildAct") << "No preset found for intro clip " << state.clip.getLinkName();
	}
	
    //The run now listens to act events and is updated via them
	state.freeTopic = false;
	int failedTopicStreak = 0;
    while(state.topicNum <= maxTopicsPerAct){
		
		///At this point in the loop we have just added a new clip and updated the duration
		//we may have been forced into a new topic area from the last part
		//we might have a visual system running also
		
		if(state.freeTopic){
			
			//TODO: Need to make sure we dont' get stuck in some topicless death loop
			if(state.timesOnCurrentTopic > 0){
				if(state.topicNum == maxTopicsPerAct){
					break;
				}
				failedTopicStreak = 0;
				state.topicNum++;
			}
			else{
				failedTopicStreak++;
				if(bLogTopicDetails) state.log << state.duration << "\tTOPIC " + state.topic << " Failed to find clips, streak " << failedTopicStreak << endl;
				if(failedTopicStreak > 3){
					break;
				}
			}
			
			state.log << state.duration << "\tCHOOSING NEW TOPIC " << state.topicNum << "/" << maxTopicsPerAct << endl;

			string newTopic;
			if(state.topicNum == maxTopicsPerAct){
				if(bLogTopicDetails) state.log << state.duration << "\t\tgoing back to seed topic " << seedTopic << endl;
				newTopic = seedTopic;
			}
			else{
				//let's pick a new topic based on the current topic
				newTopic = selectTopic(state);
				if(newTopic == state.topic){
					//We landed on the same topic, we are totally stuck -- end the act!
					state.log << state.duration << "\tERRROR: Encountered a TOPIC dead end on topic" << state.topic << endl;
					break;
				}
			}
			
			string oldTopic = state.topic;
			state.freeTopic = false;
			state.topic = newTopic;
			state.timesOnCurrentTopic = 0;
			state.topicHistory.push_back(state.topic);
			state.act->addNote(oldTopic + " -> " + newTopic, state.duration);
			
			state.log << state.duration << "\t\tPicked new topic: " << ofToUpper(state.topic)  << endl;

		}
		
		state.log << state.duration << "\tCHOOSING NEW CLIP " << endl;
		//PICK A CLIP
		vector<CloudsClip> questionClips; //possible questions this clip raises (ie other adjascent clips)
		CloudsClip nextClip = selectClip(state, questionClips);
		
		//Reject if we are the same clip as before
		if( nextClip.getID() == state.clip.getID()){
			state.log << state.duration << "\t\tERROR: Failed to find a new clip, freeing topic" << endl;			
			state.freeTopic = true;
			continue;
		}
		
		//accept the next clip
		state.clip = nextClip;
		
        ///////////////// QUESTIONS
        //adding all option clips with questions
		if(state.topicNum > 1 && state.topicNum < maxTopicsPerAct){
			addQuestions(state, questionClips);
        }
        /////////////////
		
		///////////////// DIOCHOTOMIES
        updateDichotomies(state.clip);
		/////////////////
		
		///////////////// update balances
		if(state.clip.getSpeakerGender() == "male"){
			state.moreMenThanWomen++;
		}
		else{
			state.moreMenThanWomen--;
		}
		
		state.timesOnCurrentTopic++;
		
		state.log << state.duration << "\t\tChose Clip \"" << nextClip.getLinkName() << "\" for topic \"" << state.topic << "\" ("<< state.timesOnCurrentTopic << "/" << maxTimesOnTopic << ")" << endl;
		
		//If we chose a digressing clip, through a link that didn't share our topic
		//play the clip but free the topic for the next round
		if(!state.clip.hasKeyword(state.topic) ){
			state.log << state.duration << "\t\t\tERROR: We took a digression! Freeing topic" << endl;
			state.freeTopic = true;
		}
		
		//Topic change if we reach max
		if(state.timesOnCurrentTopic == maxTimesOnTopic){
			state.log << state.duration << "\t\t\tLast clip in topic! Freeing topic" << endl;
			state.freeTopic = true;
		}

		///add the clip!
		//BASED ON CURRENT VS, ETC DECIDE HOW TO PLACE IT IN TIME AFTER PREVIOUS CLIP
		float clipStartTime = state.duration + (state.visualSystemRunning ? voClipGapTime : clipGapTime);
		state.duration = state.act->addClip(state.clip,
																 state.topic,
																 clipStartTime,
																 dichotomies);
		
		state.clipHistory.push_back(state.clip);
		
		// BASED ON CURRENT CLIP, DECIDE VS
		// consider if we are at the end of topic
		
		//TODO: START A MANDATORY VS IF WE ARE IN FREE TOPIC
		
		if(state.visualSystemRunning){
			if(bLogVisualSystemDetails) state.log << state.duration << "\tVISUALS Currently running \"" << state.preset.getID() << "\" time [" << state.visualSystemStartTime << " - " << state.visualSystemEndTime << "] " << (state.preset.indefinite ? "indefinite" : "definite") << endl;

			///should the preset end in this clip?
			//	if so commit the preset during the clip
			//	if not keep going
			//the current duration is BEFORE the new clip has been committed
			if(state.duration > state.visualSystemEndTime){
				if(bLogVisualSystemDetails) state.log << state.duration << "\t\tPassed predicted time " << state.visualSystemEndTime;
								
				//if this clip freed the topic extend our visuals out over the end of the clip a ways
				if(state.freeTopic){

					if(state.preset.indefinite){
						state.visualSystemEndTime = state.duration + visualSystemTopicEndExtend;
					}
					else {
						state.visualSystemEndTime = MIN(state.duration + visualSystemTopicEndExtend,
															   state.visualSystemStartTime + state.preset.duration);
					}
					
					state.act->addNote("Extend VS Start", state.visualSystemStartTime);
					state.act->addNote("Extend VS End", state.visualSystemEndTime);
					
					state.duration = state.act->addVisualSystem(state.preset,
																					 state.visualSystemStartTime,
																					 state.visualSystemEndTime);
					state.presetHistory.push_back(state.preset.getID());
					
					if(bLogVisualSystemDetails)state.log << " - FREE TOPIC Extending VS end time to " << state.visualSystemEndTime << " past the end of " << state.clip.getLinkName() << endl;

				}
				//else correct the end time to fit with the clip we just added
				else {
					//VO should be entirely covered. This may cause a rare problem with definite clips that don't cover the whole VO
					if(state.clip.voiceOverAudio){
						state.visualSystemEndTime = state.duration;
						if(bLogVisualSystemDetails) state.log << state.duration << "- moving end time over top of VO " << state.clip.getLinkName() << endl;
					}
					//end within the clip
					else{
						state.visualSystemEndTime = state.duration - state.clip.getDuration() / 2.;
						if(bLogVisualSystemDetails) state.log << " - moving end time to middle of " << state.clip.getLinkName() << ", time " << state.visualSystemEndTime << endl;
					}
					
					state.act->addVisualSystem(state.preset,
													  state.visualSystemStartTime,
													  state.visualSystemEndTime);
					state.presetHistory.push_back( state.preset.getID() );
				}
				
				state.visualSystemRunning = false;
			}

			//still going, extend the visual out for longer over the clip
		}
		else {
			if(bLogVisualSystemDetails) state.log << state.duration << "\tVISUALS Not run in " << state.duration - state.visualSystemEndTime << " seconds" << endl;

			//Have we waited long enough to see a visual?
			// if so do we have a good visual for this clip?
			//	  if so start it up!
			//	  if it has sound, cue the sound to end and start the visual at the end of the clip, push the duration forward
			//    otherwise start it inside of our clip and keep going
			
			if(state.duration - state.visualSystemEndTime > maxVisualSystemGapTime){
				CloudsVisualSystemPreset nextPreset = selectVisualSystem(state, true);
				if(!nextPreset.randomlySelected){
						
					state.preset = nextPreset;
					
					
					//if we have sound schedule this at the end of the clip and add it immediately to ensure no VO comes in
					//COMPUTE START TIME
					//step back into the clip
                    //TODO: respect VO & SOUND
					if(state.preset.hasSound()){
						state.visualSystemStartTime = state.duration;
					}
					else{
						state.visualSystemStartTime = state.duration - state.clip.getDuration() / 2.0;
					}
					
					//COMPUTE END TIME
					if(nextPreset.indefinite){
						state.visualSystemEndTime = state.visualSystemStartTime + maxVisualSystemRunTime;
					}
					else {
						state.visualSystemEndTime =
							state.visualSystemStartTime + MIN(maxVisualSystemRunTime, state.preset.duration);
					}
					//TODO: RESPECT HAS SOUND
					if(state.preset.hasSound()){
						//commit the visual system
						//setting the current duration here ensures the next clip starts after this one is over
						state.act->addNote("Sound VS Start", state.duration);
						state.duration = state.act->addVisualSystem(state.preset,
																						 state.visualSystemStartTime,
																						 state.visualSystemEndTime);
						state.act->addNote("Sound VS End", state.duration);
						state.presetHistory.push_back(state.preset.getID());
						state.log << state.duration << "\t\tChose new AUDIO Preset " << state.preset.getID() << "[" << state.visualSystemStartTime << " - " << state.visualSystemEndTime << "] - Pushing All clips forward" << endl;
					}
					//run it indefinitely with clips underneath
					else {
						state.visualSystemRunning = true;
						
						if(bLogVisualSystemDetails)state.log << state.duration << "\t\tChose new Preset " << state.preset.getID() << " time [" << state.visualSystemStartTime << " - " << state.visualSystemEndTime << "] " << (state.preset.indefinite ? "indefinite" : "definite") << endl;
					}
				}
				else{
					state.act->addNote("Failed VS", state.duration - state.clip.getDuration());
					if(bLogVisualSystemDetails) state.log << state.duration << "\t\tERROR Failed to find preset on current topic " << state.topic << " with clip " << state.clip.getLinkName() << endl;
				}
			}
		}		
	}
	
	state.log << state.duration << "\tACT ENDED on clip " << state.clip.getLinkName() << " explored topics " << state.topicNum << "/" << maxTopicsPerAct << " with " << state.timesOnCurrentTopic << " clips on final topic \"" << state.topic << "\"" << endl;
	
	if(state.visualSystemRunning){
		state.duration = state.act->addVisualSystem(state.preset,
																		 state.visualSystemStartTime,
																		 state.visualSystemEndTime);

		if(bLogVisualSystemDetails) state.log << state.duration << " Commiting final preset " << state.preset.getID() << " at the end of the act " <<endl;
	}

	char logpath[1024];
	sprintf(logpath, "%slogs/acts/ACT_%d_%d_%d_%d.txt", GetCloudsDataPath().c_str(), ofGetMonth(), ofGetDay(), ofGetMinutes(), ofGetSeconds() );
	ofBuffer logbuf(state.log.str());
	ofBufferToFile( logpath, logbuf );

//	cout << state.log.str() << endl;
	
	state.act->populateTime();

    CloudsActEventArgs args(state.act);
    ofNotifyEvent(events.actCreated, args);
    
    return state.act;
}

CloudsClip CloudsStoryEngine::selectClip(CloudsStoryState& state, vector<CloudsClip>& questionClips){
	vector<CloudsClip> nextOptions;
	//conclusion clips if we are on the last topic
	if(state.topicNum == maxTopicsPerAct){
		nextOptions = parser->getClipsWithKeyword(state.topic);
		
		if(bLogClipDetails)state.log << state.duration << "\t\tConclusion of \"" << state.topic << "\", selecting from " << nextOptions.size() << " clips " << endl;
	}
	else{
		nextOptions = parser->getClipsWithKeyword(state.clip.getKeywords());
	}
	
	vector<CloudsLink>& links = parser->getLinksForClip( state.clip );
	for(int i = 0; i < links.size(); i++){
		bool valid = true;
		for(int c = 0; c < nextOptions.size(); c++){
			if(nextOptions[c].getLinkName() == links[i].targetName){
				//don't add clips that are already included
				valid = false;
				break;
			}
		}
		
		if(valid){
			nextOptions.push_back( parser->getClipWithLinkName(links[i].targetName) );
		}
	}

	//remove suppressions
	vector<CloudsLink>& suppressions = parser->getSuppressionsForClip( state.clip );
	for(int i = 0; i < suppressions.size(); i++){
		for(int nt = nextOptions.size()-1; nt >= 0; nt--){
			if(nextOptions[nt].getLinkName() == suppressions[i].targetName){
				nextOptions.erase(nextOptions.begin() + nt);
			}
		}
	}
	
	if(bLogClipDetails)state.log << state.duration << "\t\tCurrent clip " << state.clip.getLinkName() << " has " << state.clip.getKeywords().size() << "\" keywords, selecting a new clip from " << nextOptions.size() << " related options" << endl;

	/////////////////SELECTION
	float topScore = 0;
	vector< pair<float, string> > cliplogs;
	for(int i = 0; i < nextOptions.size(); i++){

		CloudsClip& nextClipOption = nextOptions[ i ];
		
		stringstream cliplog;
		cliplog << state.duration << "\t\t\t" << nextClipOption.getLinkName() << endl;
		float score = scoreForClip(state, nextClipOption, cliplog);
		topScore = MAX(topScore, score);
		cliplogs.push_back(make_pair(score, cliplog.str()));
		
		nextClipOption.currentScore = score;
	}
	
	
	if(bLogClipDetails){
		sort(cliplogs.begin(), cliplogs.end(), logsort);
		for(int i = 0; i < cliplogs.size(); i++){
			state.log << cliplogs[i].second;
		}
	}
	
	if(topScore == 0){
		//Dead end!  start over with a free topic and see what happens
		state.log << state.duration << "\t\tERROR: Hit dead end with current topic \"" << state.topic << "\""
				<< " out of " << nextOptions.size() << " potential clips. " 
				<< " played " << state.timesOnCurrentTopic << " times. "
			<< " Act topic " << state.topicNum << "/" << maxTimesOnTopic << endl;
		return state.clip;
	}
	
	vector<CloudsClip> winningClips;
	for(int i = 0; i < nextOptions.size(); i++){
		if(nextOptions[i].currentScore == topScore){
			winningClips.push_back(nextOptions[i]);
		}
	}
	
	//select next clip
	CloudsClip& winningClip = winningClips[ofRandom(winningClips.size())];
	
	//select next questions
	for(int k = 0; k < nextOptions.size(); k++){
		if(nextOptions[k].hasQuestion()){
			questionClips.push_back(nextOptions[k]);
		}
	}
	return winningClip;
}

#pragma mark VISUAL SYSTEMS
CloudsVisualSystemPreset CloudsStoryEngine::selectVisualSystem(CloudsStoryState& state, bool allowSound){

    vector<CloudsVisualSystemPreset> presets = visualSystems->getPresetsForKeywords(state.clip.getKeywords(),
																					state.clip.getLinkName());
    CloudsVisualSystemPreset preset;
    float topScore = 0;
	for(int i = 0; i < presets.size(); i++){
		string presetLog;
        //TODO: RESPECT ALLOW VO
		if(presets[i].hasSound() && !allowSound){
			continue;
		}
		presets[i].currentScore = scoreForVisualSystem(state, presets[i]);
		topScore = MAX(presets[i].currentScore, topScore);
	}
	
	if(topScore != 0){
        vector<CloudsVisualSystemPreset> winningPresets;
        for(int i = 0; i < presets.size(); i++){
            if(presets[i].currentScore == topScore){
                winningPresets.push_back(presets[i]);
            }
        }
        preset = winningPresets[ ofRandom(winningPresets.size()) ];
        preset.randomlySelected = false;
    }
    else{
		preset.system = visualSystems->getEmptySystem(state.topic,
													  state.clip.getKeywords());
		preset.systemName = "MISSING";
		preset.presetName = state.topic;
        preset.randomlySelected = true;
		preset.missingContent = true;
        log += ",ERROR,no presets found! " + state.topic + "\n";
    }
	return preset;
}

//TODO: Add to main logger
float CloudsStoryEngine::scoreForVisualSystem(CloudsStoryState& state, CloudsVisualSystemPreset& potentialNextPreset)
{
    log += ",,"+potentialNextPreset.getID() + ",";
	
    if(!potentialNextPreset.enabled){
        log += "rejected because it's disabled";
        return 0;
    }
    
    if(visualSystems->isClipSuppressed(potentialNextPreset.getID(), state.clip.getLinkName())){
        log += "rejected because the system is suppressed for this clip";
        return 0;
    }
    
    if(ofContains(state.presetHistory, potentialNextPreset.getID())){
        log += "rejected because we've seen it before";
        return 0;
    }
    
    vector<string> keywords = visualSystems->keywordsForPreset(potentialNextPreset);
//    if(keywords.size() == 0 ){
//        log += "rejected because it has no keywords";
//        return 0;
//    }
    

#ifdef OCULUS_RIFT
	if(!potentialNextPreset.oculusCompatible){
        log += "rejected because it is not oculus compatible";
        return 0;
	}
#else
	if(potentialNextPreset.oculusCompatible){
        log += "rejected because it is for the oculus";
        return 0;
	}
#endif
	
    float mainTopicScore = 0;
    float secondaryTopicScore = 0;
	float linkedClipScore = 0;
	
    vector<string> keywordsMatched;
    for(int i = 0; i < keywords.size(); i++){
        if(keywords[i] == state.topic){
            mainTopicScore += visualSystemPrimaryTopicBoost;
            keywordsMatched.push_back(keywords[i]);
        }
        else if(ofContains(state.clip.getKeywords(), keywords[i])){
            secondaryTopicScore += visualSystemSecondaryTopicBoost;
            keywordsMatched.push_back(keywords[i]);
        }
		
		//if the clip and the preset are linked
		if(visualSystems->isClipLinked(potentialNextPreset.getID(), state.clip.getID())){
			linkedClipScore = 10;
		}
    }
	
    float totalScore = mainTopicScore + secondaryTopicScore + linkedClipScore;
    log += ofToString(totalScore,1) +","+ ofToString(mainTopicScore,1) +","+ ofToString(secondaryTopicScore,1) +","+ ofJoinString(keywordsMatched, "; ");
    return totalScore;
}

void CloudsStoryEngine::clearDichotomiesBalance(){
    for(int i = 0; i < dichotomies.size(); i++){
        dichotomies[i].balance = 0;
    }
}

void CloudsStoryEngine::addQuestions(CloudsStoryState& state, vector<CloudsClip>& questionClips){

	set<string> questionSetTopics;
	for(int k = 0; k < questionClips.size(); k++){
		
		if( historyContainsClip(questionClips[k], state.clipHistory) ){
			continue;
		}
		
		vector<string> topicsWithQuestions = questionClips[k].getTopicsWithQuestions();
		for(int t = 0; t < topicsWithQuestions.size(); t++){
			//don't add a topic more than once
			string questionTopic = topicsWithQuestions[t];
			if(questionSetTopics.find(questionTopic) == questionSetTopics.end() &&
			   !ofContains(state.topicHistory, questionTopic))
			{
				state.act->addQuestion(questionClips[k],questionTopic, state.duration);
				questionSetTopics.insert(questionTopic);
			}
		}
	}
}

void CloudsStoryEngine::updateDichotomies(CloudsClip& clip){
    vector<string> specialkeywords = clip.getSpecialKeywords();
    
    for(int i=0; i < dichotomies.size();i++){
        for(int j=0; j < specialkeywords.size();j++){
            
            if(dichotomies[i].left == specialkeywords[j]){
                dichotomies[i].balance -= 1;
                //cout<<dichotomies[i].left<<": +1"<<endl;
            }
            else if (dichotomies[i].right == specialkeywords[j]){
                //cout<<dichotomies[i].right<<": +1"<<endl;
                dichotomies[i].balance += 1;
            }
        }
    }
}

//intentionally by copy so that we can store them as we go
vector<CloudsDichotomy> CloudsStoryEngine::getCurrentDichotomyBalance(){
    return dichotomies;
}

#pragma mark TOPIC SCORES
string CloudsStoryEngine::selectTopic(CloudsStoryState& state){
    
	//TODO: Reconsider in what way we choose the next topic based on families or shared keywords
    vector<string>& topics = state.clip.getKeywords();
	//vector<string>& topics = parser->getKeywordFamily( topic );
	if(bLogTopicDetails) state.log << state.duration << "\t\tSelecting from " << topics.size() << " potential next topics on clip " << state.clip.getLinkName() << endl;

    vector<float> topicScores;
    topicScores.resize(topics.size());
    float topicHighScore = 0;
    for(int i = 0; i < topics.size(); i++){
		if(bLogTopicDetails) state.log << state.duration << "\t\t\tConsidering " << topics[i] << endl;
        topicScores[i] = scoreForTopic(state, topics[i]);
		if(bLogTopicDetails) state.log << state.duration << "\t\t\t\tScore for shared topic	" << topics[i] << " : " << topicScores[i] << endl;
        topicHighScore = MAX(topicHighScore,topicScores[i]);
    }
    
    if(topicHighScore == 0){
        //Dead end!  Let's try a surrounding topic
		vector<string>& topicFamilies = parser->getKeywordFamily( state.topic );
	    topicScores.resize(topicFamilies.size());
		if(bLogTopicDetails) state.log << state.duration << "\t\tERROR TOPIC DEAD END Trying family of " << state.topic << " with " <<  topicFamilies.size() << " topics" << endl;
		for(int i = 0; i < topicFamilies.size(); i++){
			topicScores[i] = scoreForTopic(state, topicFamilies[i]);
			state.log << state.duration << "\t\t\tScore for family member	" << topicFamilies[i] << " : " <<  topicScores[i] << endl;
			topicHighScore = MAX(topicHighScore,topicScores[i]);
		}
		
		//still!? then it's a dead end
		if(topicHighScore == 0){
			if(bLogTopicDetails) state.log << state.duration << "\t\tERROR Even our family couldn't help us move on from topic " << state.topic << " on clip " <<  state.clip.getLinkName() << endl;
			return state.topic;
		}
    }
    
    vector<string> winningTopics;
    for(int i = 0; i < topics.size(); i++){
        if(topicScores[i] == topicHighScore){
            winningTopics.push_back(topics[i]);
        }
    }
	
    string newTopic = winningTopics[ ofRandom(winningTopics.size()) ];
	if(bLogTopicDetails) state.log << state.duration << "\t\tTOPIC " << newTopic << " won with score " << topicHighScore << endl;
    return newTopic;
}

float CloudsStoryEngine::scoreForTopic(CloudsStoryState& state, string potentialNextTopic)
{
    stringstream logstr;
    if(state.topic == potentialNextTopic){
		if(bLogTopicDetails)state.log << state.duration << "\t\t\t\tREJECTED Topic " << potentialNextTopic << ", same as current topic" << endl;
        return 0;
    }
    
    if(ofContains(state.topicHistory, potentialNextTopic) && !state.topicNum == maxTopicsPerAct-1){
		if(bLogTopicDetails)state.log << state.duration << "\t\t\t\t\tREJECTED Topic " << potentialNextTopic << " has already been explored" << endl;
        return 0;
    }
    
    float score = 0;
    float lastClipCommonality = 0;
    float twoClipsAgoCommonality = 0;
    if(state.clipHistory.size() > 1 &&
	   ofContains( state.clipHistory[state.clipHistory.size()-2].getKeywords(), potentialNextTopic) )
	{
        lastClipCommonality = lastClipSharesTopicBoost;
    }
    
    if(state.clipHistory.size() > 2 &&
	   ofContains( state.clipHistory[state.clipHistory.size()-3].getKeywords(), potentialNextTopic) )
	{
        twoClipsAgoCommonality = twoClipsAgoSharesTopicBoost;
    }
    
    int sharedClips = parser->getNumberOfSharedClips(state.topic,potentialNextTopic);
    int totalClips = parser->getNumberOfClipsWithKeyword(potentialNextTopic);
    float relevancyScore = (1.0 * sharedClips / totalClips) * topicRelevancyMultiplier;

    //Cohesion is not being used
//    float distanceBetweenKeywords = parser->getDistanceFromAdjacentKeywords(currentTopic, newTopic);
//    float cohesionIndexForKeywords = parser->getCohesionIndexForKeyword(newTopic);
//    float cohesionScore = 0; //TODO:Add cohesion score to favor nearby topics
    
    score  = lastClipCommonality + twoClipsAgoCommonality + relevancyScore;//  +   cohesionScore;

    return score;
}


#pragma mark CLIP SCORES
float CloudsStoryEngine::scoreForClip(CloudsStoryState& state, CloudsClip& potentialNextClip, stringstream& cliplog){
    
    //rejection criteria -- flat out reject clips on some basis
    if(combinedClipsOnly && !potentialNextClip.hasMediaAsset){
		cliplog << state.duration << "\t\t\t\t\tREJECTED Clip \"" << potentialNextClip.getLinkName() << "\" no combined video file" << endl;
        return 0;
    }
        
    bool link = parser->clipLinksTo( state.clip.getLinkName(), potentialNextClip.getLinkName() );
    if(!link && potentialNextClip.person == state.clip.person){
		cliplog << state.duration << "\t\t\t\t\tREJECTED Clip " << potentialNextClip.getLinkName() << ": same person" << endl;
        return 0;
    }
    
    //reject any nodes we've seen already
    if(historyContainsClip(potentialNextClip, state.clipHistory)){ //TODO discourage...
		cliplog << state.duration << "\t\t\t\t\tREJECTED Clip " << potentialNextClip.getLinkName() << ": already visited" << endl;
        return 0;
    }
    
    //TODO: make this smarter
    int occurrences = occurrencesOfPerson(potentialNextClip.person, 20, state.clipHistory);
    if(occurrences > 4){
		cliplog << state.duration << "\t\t\t\t\tREJECTED Clip " << potentialNextClip.getLinkName() << ": person appeared more than 4 times in the last 20 clips" << endl; 
        return 0;
    }
    
	//////TODO: RECONSIDER THIS BASED ON NEW IDEAS
    //If a VS is not running reject clips that do not have video footage
    if((potentialNextClip.hasSpecialKeyword("#vo") || potentialNextClip.voiceOverAudio) &&
	   !state.visualSystemRunning)
	{
		cliplog << state.duration << "\t\t\t\t\tREJECTED Clip: VO without Visual System" << endl;
        return 0;
    }
    
    //If a VS is running and is of a definite duration do not use voice over clips
	//TODO: Check the durations, mabye its OK
    if( (potentialNextClip.hasSpecialKeyword("#vo") || potentialNextClip.voiceOverAudio) &&
	   state.visualSystemRunning &&
	   !state.preset.indefinite)
	{
		cliplog << state.duration << "\t\t\t\t\tREJECTED Clip: VO without flexible Visual System" << endl;
        return 0;
    }
    
    // Ignore clip if it has been tagged as a  #dud
    if(potentialNextClip.hasSpecialKeyword("#dud")){
        cliplog << state.duration << "\t\t\t\t\tREJECTED Clip: this clip has been marked with the #dud keyword"<<endl;
        return 0;
    }
    
    //  can't get a #hard clip until the topic has been heard 2 times by any clip
	//TODO: RE ADD RULE?
//    if (numTopicHistoryOccurrences < 2 && potentialNextClip.hasSpecialKeyword("#hard")  ) {
//		return 0;
//    }
    
    if(!ofContains(potentialNextClip.getKeywords(), state.topic) &&
	   state.timesOnCurrentTopic < minTimesOnTopic)
	{
        cliplog << state.duration << "\t\t\t\t\tREJECTED Clip: this clip is a premature digression" << endl;
		return 0;
	}
	
	string clipDifficulty = "medium";
    if (potentialNextClip.hasSpecialKeyword("#easy")) {
        clipDifficulty = "easy";
    }
    else if(potentialNextClip.hasSpecialKeyword("#hard")){
        clipDifficulty = "hard";
    }
	
	if(state.run == 0 && clipDifficulty != "easy"){
        cliplog << state.duration << "\t\t\t\t\tREJECTED Clip: only easy clips in the intro please!!" << endl;
		return 0;
	}
	
	if(state.run < 1 && clipDifficulty == "medium"){
        cliplog << state.duration << "\t\t\t\t\tREJECTED Clip: medium difficulty in first act" << endl;
		return 0;
	}

	if(state.run < 2 && clipDifficulty == "hard" ){
        cliplog << state.duration << "\t\t\t\t\tREJECTED Clip: hard clips come 3rd act" << endl;
		return 0;
	}

    //Base score
    float totalScore = 0;
    float offTopicScore = 0; //negative if this is a link & off topic
    float topicsInCommonScore = 0;
    float topicsInCommonWithPreviousScore = 0;
    float topicsInCommonWithPreviousAveraged = 0;
    float linkScore = 0;
    float samePersonOccuranceScore = 0;
    float dichotomiesScore = 0;
    float voiceOverScore = 0;
    float genderBalanceScore = 0;
    float distantClipSuppressionScore = 0;
    float goldClipScore = 0;
    float easyClipScore = 0;
    
    //need to consider discouraging clips with a lot of topics
    float topicsInCommon = parser->getSharedKeywords(state.clip, potentialNextClip).size();
    topicsInCommonScore += topicsInCommon * topicsInCommonMultiplier;
    
    if(state.clipHistory.size() > 1){
		CloudsClip& twoClipsAgo = state.clipHistory[state.clipHistory.size()-2];
		
		//TODO: Consider Normalizing
        float topicsInCommonWithPrevious = parser->getSharedKeywords(twoClipsAgo, potentialNextClip).size();
		topicsInCommonWithPreviousScore += topicsInCommonWithPrevious * topicsinCommonWithPreviousMultiplier;
		
		//like this....
//        topicsInCommonWithPreviousAveraged = (parser->getNumberOfSharedKeywords(history[history.size()-2], potentialNextClip) /  potentialNextClip.getKeywords().size() );
    }
    
    if( link ){
        linkScore += linkFactor;
    }
    
    //penalize for the person occurring
    //TODO: make this a little smarter
    samePersonOccuranceScore = -occurrences * samePersonOccurrenceSuppressionFactor;
    
    //history should contain #keywords dichotomies, and then augment score
    vector<string> specialKeywords = potentialNextClip.getSpecialKeywords();
    
    for(int i = 0; i < dichotomies.size(); i++){
        for( int k = 0; k < specialKeywords.size(); k++){
            if(dichotomies[i].right == specialKeywords[k]){
                
                if(dichotomies[i].balance > dichotomyThreshold){
                    dichotomies[i].balance = -1;
                }
                else{
                    dichotomiesScore += dichotomies[i].balance * dichotomyWeight;
                }
                
            }
            else if(dichotomies[i].left == specialKeywords[k]){
                
                if(dichotomies[i].balance < -dichotomyThreshold){
                    dichotomies[i].balance = 1;
                }
                else{
                    dichotomiesScore += -dichotomies[i].balance * dichotomyWeight;
                }
            }
        }
    }
    
    if( state.visualSystemRunning && potentialNextClip.hasSpecialKeyword("#vo") ){
        //TODO: Make voice over score less arbitrary
        voiceOverScore = 15;
    }
    
    //gender balance scorec
    genderBalanceScore = (potentialNextClip.getSpeakerGender() == "male" ? -1 : 1 ) * genderBalanceFactor * state.moreMenThanWomen;
    
    //discourage distant clips
    distantClipSuppressionScore = -state.clip.networkPosition.distance( potentialNextClip.networkPosition ) * distantClipSuppressionFactor;
    
    //gold clip score
    goldClipScore = (potentialNextClip.hasSpecialKeyword("#gold") ? goldClipFactor : 0);
    
    //if topic is unbreached boost easy clips
    if(state.timesOnCurrentTopic < 1 && potentialNextClip.hasSpecialKeyword("#easy")){
        easyClipScore  = easyClipScoreFactor;
    }
	
	///TODO CONCLUSION
    
    //ADD IT UP
    totalScore = linkScore +
					topicsInCommonScore +
					topicsInCommonWithPreviousScore +
//					offTopicScore +
					samePersonOccuranceScore +
					dichotomiesScore +
					genderBalanceScore +
//					distantClipSuppressionScore +
					voiceOverScore +
					goldClipScore +
					easyClipScore;
					
    stringstream ss;
    string linkName = potentialNextClip.getLinkName();
    ofStringReplace(linkName, ",", ":");
    
	cliplog << state.duration << "\t\t\t\t\tClip Link to Curr\t" << linkScore << endl;
	cliplog << state.duration << "\t\t\t\t\tCommon Topics Curr\t" << topicsInCommonScore << endl;
	cliplog << state.duration << "\t\t\t\t\tCommon Topics Prev\t" << topicsInCommonWithPreviousScore << endl;
	cliplog << state.duration << "\t\t\t\t\tLoud Mouth Suppress\t" << samePersonOccuranceScore << endl;
	cliplog << state.duration << "\t\t\t\t\tGender Balance\t\t" << genderBalanceScore << endl;
	cliplog << state.duration << "\t\t\t\t\tDichotomies Weight\t" << dichotomiesScore << endl;
	cliplog << state.duration << "\t\t\t\t\tVoice Over Bump\t\t" << voiceOverScore << endl;
	cliplog << state.duration << "\t\t\t\t\tGold Clip Bump\t\t" << goldClipScore << endl;
	cliplog << state.duration << "\t\t\t\t\tEasy Clip Score\t\t" << easyClipScore << endl;
	cliplog << state.duration << "\t\t\t\t" << potentialNextClip.getLinkName() <<  " TOTAL " << totalScore << endl;

    return totalScore;
}


////TODO: timing of transitions per clip based on clip length
//float CloudsStoryEngine::getHandleForClip(CloudsClip& clip){
//    // if clip is longer than minimum length for long clip allow the 2 second intro
//    //        if (clip.getDuration()>minClipDurationForStartingOffset) {
//    //                return 0;
//    //        }
//    return 1; //temp just 1 for now
//}

bool CloudsStoryEngine::historyContainsClip(CloudsClip& m, vector<CloudsClip>& history){
    string clipID = m.getID();
    
    for(int i = 0; i < history.size(); i++){
		
        if(clipID == history[i].getID()){
            return true;
        }
        
        if (m.overlapsWithClip(history[i])) {
//			cout << "        REJECTED Clip " << m.getLinkName() << ": it overlaps with clip " <<history[i].getLinkName()<<" which has already been visited"<<endl;
			return true;
        }
    }
    return false;
}

int CloudsStoryEngine::occurrencesOfPerson(string person, int stepsBack, vector<CloudsClip>& history){
    int occurrences = 0;
    int startPoint = history.size() - MIN(stepsBack, history.size() );
    //        cout << "finding occurrences ... " << person << " " << clipHistory.size() << " steps back " << stepsBack << " start point " << startPoint << "/" << clipHistory.size() << endl;
    for(int i = startPoint; i < history.size()-1; i++){ // -1 because the current clip is part of history
        //                cout << "COMPARING " << clipHistory[i].person << " to " << person << endl;
        if(history[i].person == person){
            occurrences++;
        }
    }
    return occurrences;
}

CloudsStoryEvents& CloudsStoryEngine::getEvents(){
    return events;
}