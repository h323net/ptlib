/*
 * vxml.h
 *
 * VXML engine for pwlib library
 *
 * Copyright (C) 2002 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision: 32948 $
 * $Author: rjongbloed $
 * $Date: 2014-10-10 00:30:31 +0100 (Fri, 10 Oct 2014) $
 */

#ifndef PTLIB_VXML_H
#define PTLIB_VXML_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


#include <ptclib/pxml.h>

#if P_VXML

#include <ptlib/pfactory.h>
#include <ptlib/pipechan.h>
#include <ptclib/delaychan.h>
#include <ptclib/pwavfile.h>
#include <ptclib/ptts.h>
#include <ptclib/url.h>

#include <queue>


class PVXMLSession;
class PVXMLDialog;
class PVXMLSession;

// these are the same strings as the Opal equivalents, but as this is PWLib, we can't use Opal contants
#define VXML_PCM16         "PCM-16"
#define VXML_G7231         "G.723.1"
#define VXML_G729          "G.729"


//////////////////////////////////////////////////////////////////

class PVXMLGrammar : public PObject
{
  PCLASSINFO(PVXMLGrammar, PObject);
  public:
    PVXMLGrammar(PVXMLSession & session, PXMLElement & field);

    virtual void OnUserInput(const char ch) = 0;
    virtual void Start();
    virtual bool Process();

    enum GrammarState {
      Idle,         ///< Not yet started
      Started,      ///< Grammar awaiting input
      Filled,       ///< got something that matched the grammar
      NoInput,      ///< timeout or still waiting to match
      NoMatch,      ///< recognized something but didn't match the grammar
      Help          ///< help keyword
    };

    GrammarState GetState() const { return m_state; }

    void SetSessionTimeout();
    void SetTimeout(const PTimeInterval & timeout);

  protected:
    PDECLARE_NOTIFIER(PTimer, PVXMLGrammar, OnTimeout);

    PVXMLSession & m_session;
    PXMLElement  & m_field;
    PString        m_value;
    GrammarState   m_state;
    PTimeInterval  m_timeout;
    PTimer         m_timer;
    PMutex         m_mutex;
};


//////////////////////////////////////////////////////////////////

class PVXMLMenuGrammar : public PVXMLGrammar
{
  PCLASSINFO(PVXMLMenuGrammar, PVXMLGrammar);
  public:
    PVXMLMenuGrammar(PVXMLSession & session, PXMLElement & field);
    virtual void OnUserInput(const char ch);
    virtual bool Process();
};


//////////////////////////////////////////////////////////////////

class PVXMLDigitsGrammar : public PVXMLGrammar
{
  PCLASSINFO(PVXMLDigitsGrammar, PVXMLGrammar);
  public:
    PVXMLDigitsGrammar(
      PVXMLSession & session,
      PXMLElement & field,
      PINDEX minDigits,
      PINDEX maxDigits,
      PString terminators
    );

    virtual void OnUserInput(const char ch);

  protected:
    PINDEX  m_minDigits;
    PINDEX  m_maxDigits;
    PString m_terminators;
};


//////////////////////////////////////////////////////////////////

class PVXMLCache : public PSafeObject
{
  public:
    PVXMLCache();

    virtual bool Get(
      const PString & prefix,
      const PString & key,
      const PString & fileType,
      PFilePath     & filename
    );

    virtual bool PutWithLock(
      const PString   & prefix,
      const PString   & key,
      const PString   & fileType,
      PFile           & file
    );

    void SetDirectory(
      const PDirectory & directory
    );

    const PDirectory & GetDirectory() const
    { return m_directory; }

  protected:
    virtual PFilePath CreateFilename(
      const PString & prefix,
      const PString & key,
      const PString & fileType
    );

    PDirectory m_directory;
};

//////////////////////////////////////////////////////////////////

class PVXMLChannel;

class PVXMLSession : public PIndirectChannel
{
  PCLASSINFO(PVXMLSession, PIndirectChannel);
  public:
    PVXMLSession(PTextToSpeech * tts = NULL, PBoolean autoDelete = false);
    virtual ~PVXMLSession();

    // new functions
    PTextToSpeech * SetTextToSpeech(PTextToSpeech * tts, PBoolean autoDelete = false);
    PTextToSpeech * SetTextToSpeech(const PString & ttsName);
    PTextToSpeech * GetTextToSpeech() const { return m_textToSpeech; }

    void SetCache(PVXMLCache & cache);
    PVXMLCache & GetCache();

    void SetRecordDirectory(const PDirectory & dir) { m_recordDirectory = dir; }
    const PDirectory & GetRecordDirectory() const { return m_recordDirectory; }

    virtual PBoolean Load(const PString & source);
    virtual PBoolean LoadFile(const PFilePath & file, const PString & firstForm = PString::Empty());
    virtual PBoolean LoadURL(const PURL & url);
    virtual PBoolean LoadVXML(const PString & xml, const PString & firstForm = PString::Empty());
    virtual PBoolean IsLoaded() const { return m_xml.IsLoaded(); }

    virtual PBoolean Open(const PString & mediaFormat);
    virtual PBoolean Close();

    virtual PBoolean Execute();

    PVXMLChannel * GetAndLockVXMLChannel();
    void UnLockVXMLChannel() { m_sessionMutex.Signal(); }
    PMutex & GetSessionMutex() { return m_sessionMutex; }

    virtual PBoolean LoadGrammar(PVXMLGrammar * grammar);

    virtual PBoolean PlayText(const PString & text, PTextToSpeech::TextType type = PTextToSpeech::Default, PINDEX repeat = 1, PINDEX delay = 0);

    virtual PBoolean PlayFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0, PBoolean autoDelete = false);
    virtual PBoolean PlayData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);
    virtual PBoolean PlayCommand(const PString & data, PINDEX repeat = 1, PINDEX delay = 0);
    virtual PBoolean PlayResource(const PURL & url, PINDEX repeat = 1, PINDEX delay = 0);
    virtual PBoolean PlayTone(const PString & toneSpec, PINDEX repeat = 1, PINDEX delay = 0);
    virtual PBoolean PlayElement(PXMLElement & element);

    //virtual PBoolean PlayMedia(const PURL & url, PINDEX repeat = 1, PINDEX delay = 0);
    virtual PBoolean PlaySilence(PINDEX msecs = 0);
    virtual PBoolean PlaySilence(const PTimeInterval & timeout);

    virtual PBoolean PlayStop();

    virtual void SetPause(PBoolean pause);
    virtual void GetBeepData(PBYTEArray & data, unsigned ms);

    virtual void OnUserInput(const PString & str);

    PString GetXMLError() const;

    virtual void OnEndDialog();
    virtual void OnEndSession();

    enum TransferType {
      BridgedTransfer,
      BlindTransfer,
      ConsultationTransfer
    };
    virtual bool OnTransfer(const PString & /*destination*/, TransferType /*type*/) { return false; }
    void SetTransferComplete(bool state);

    const PStringToString & GetVariables() { return m_variables; }
    virtual PCaselessString GetVar(const PString & str) const;
    virtual void SetVar(const PString & ostr, const PString & val);
    virtual PString EvaluateExpr(const PString & oexpr);

    static PTimeInterval StringToTime(const PString & str, int dflt = 0);

    PDECLARE_NOTIFIER(PThread, PVXMLSession, VXMLExecute);

    bool SetCurrentForm(const PString & id, bool fullURI);
    bool GoToEventHandler(PXMLElement & element, const PString & eventName);

    // overrides from VXMLChannelInterface
    virtual void OnEndRecording(PINDEX bytesRecorded, bool timedOut);
    virtual void Trigger();


    virtual PBoolean TraverseAudio(PXMLElement & element);
    virtual PBoolean TraverseBreak(PXMLElement & element);
    virtual PBoolean TraverseValue(PXMLElement & element);
    virtual PBoolean TraverseSayAs(PXMLElement & element);
    virtual PBoolean TraverseGoto(PXMLElement & element);
    virtual PBoolean TraverseGrammar(PXMLElement & element);
    virtual PBoolean TraverseRecord(PXMLElement & element);
    virtual PBoolean TraversedRecord(PXMLElement & element);
    virtual PBoolean TraverseIf(PXMLElement & element);
    virtual PBoolean TraverseExit(PXMLElement & element);
    virtual PBoolean TraverseVar(PXMLElement & element);
    virtual PBoolean TraverseSubmit(PXMLElement & element);
    virtual PBoolean TraverseMenu(PXMLElement & element);
    virtual PBoolean TraversedMenu(PXMLElement & element);
    virtual PBoolean TraverseChoice(PXMLElement & element);
    virtual PBoolean TraverseProperty(PXMLElement & element);
    virtual PBoolean TraverseDisconnect(PXMLElement & element);
    virtual PBoolean TraverseForm(PXMLElement & element);
    virtual PBoolean TraversedForm(PXMLElement & element);
    virtual PBoolean TraversePrompt(PXMLElement & element);
    virtual PBoolean TraversedPrompt(PXMLElement & element);
    virtual PBoolean TraverseField(PXMLElement & element);
    virtual PBoolean TraversedField(PXMLElement & element);
    virtual PBoolean TraverseTransfer(PXMLElement & element);
    virtual PBoolean TraversedTransfer(PXMLElement & element);

    __inline PVXMLChannel * GetVXMLChannel() const { return (PVXMLChannel *)readChannel; }

  protected:
    virtual bool InternalLoadVXML(const PString & xml, const PString & firstForm);

    virtual bool ProcessNode();
    virtual bool ProcessEvents();
    virtual bool ProcessGrammar();
    virtual bool NextNode(bool processChildren);
    void ClearBargeIn();
    void FlushInput();

    void SayAs(const PString & className, const PString & text);
    void SayAs(const PString & className, const PString & text, const PString & voice);

    PURL NormaliseResourceName(const PString & src);

    PMutex           m_sessionMutex;

    PURL             m_rootURL;
    PXML             m_xml;

    PTextToSpeech  * m_textToSpeech;
    PVXMLCache     * m_ttsCache;
    bool             m_autoDeleteTextToSpeech;

    PThread     *    m_vxmlThread;
    bool             m_abortVXML;
    PSyncPoint       m_waitForEvent;
    PXMLObject  *    m_currentNode;
    bool             m_xmlChanged;
    bool             m_speakNodeData;
    bool             m_bargeIn;
    bool             m_bargingIn;

    PVXMLGrammar *   m_grammar;
    char             m_defaultMenuDTMF;

    PStringToString  m_variables;
    PString          m_variableScope;

    std::queue<char> m_userInputQueue;
    PMutex           m_userInputMutex;

    enum {
      NotRecording,
      RecordingInProgress,
      RecordingComplete
    }          m_recordingStatus;
    bool       m_recordStopOnDTMF;
    PString    m_recordingName;
    PTime      m_recordingStartTime;
    PDirectory m_recordDirectory;

    enum {
      NotTransfering,
      TransferInProgress,
      TransferFailed,
      TransferSuccessful,
      TransferCompleted
    }     m_transferStatus;
    PTime m_transferStartTime;
};


//////////////////////////////////////////////////////////////////

class PVXMLRecordable : public PObject
{
  PCLASSINFO(PVXMLRecordable, PObject);
  public:
    PVXMLRecordable();

    virtual PBoolean Open(const PString & arg) = 0;

    virtual bool OnStart(PVXMLChannel & incomingChannel) = 0;
    virtual void OnStop() { }

    virtual PBoolean OnFrame(PBoolean /*isSilence*/) { return false; }

    const PTimeInterval & GetFinalSilence() const { return m_finalSilence; }
    void SetFinalSilence(const PTimeInterval & v) { m_finalSilence = v > 0 ? v : 60000; }

    const PTimeInterval & GetMaxDuration() const { return m_maxDuration; }
    void SetMaxDuration(const PTimeInterval & v) { m_maxDuration = v > 0 ? v : 86400000; }

  protected:
    PSimpleTimer  m_silenceTimer;
    PSimpleTimer  m_recordTimer;
    PTimeInterval m_finalSilence;
    PTimeInterval m_maxDuration;
};

//////////////////////////////////////////////////////////////////

class PVXMLRecordableFilename : public PVXMLRecordable
{
  PCLASSINFO(PVXMLRecordableFilename, PVXMLRecordable);
  public:
    PBoolean Open(const PString & arg);
    bool OnStart(PVXMLChannel & incomingChannel);
    PBoolean OnFrame(PBoolean isSilence);

  protected:
    PFilePath m_fileName;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayable : public PObject
{
  PCLASSINFO(PVXMLPlayable, PObject);
  public:
    PVXMLPlayable();

    virtual PBoolean Open(PVXMLChannel & chan, const PString & arg, PINDEX delay, PINDEX repeat, PBoolean autoDelete);

    virtual bool OnStart() = 0;
    virtual bool OnRepeat();
    virtual bool OnDelay();
    virtual void OnStop();

    virtual void SetRepeat(PINDEX v) 
    { m_repeat = v; }

    virtual PINDEX GetRepeat() const
    { return m_repeat; }

    virtual PINDEX GetDelay() const
    { return m_delay; }

    void SetFormat(const PString & fmt)
    { m_format = fmt; }

    void SetSampleFrequency(unsigned rate)
    { m_sampleFrequency = rate; }

    friend class PVXMLChannel;

  protected:
    PVXMLChannel * m_vxmlChannel;
    PChannel * m_subChannel;
    PINDEX   m_repeat;
    PINDEX   m_delay;
    PString  m_format;
    unsigned m_sampleFrequency;
    bool     m_autoDelete;
    bool     m_delayDone; // very tacky flag used to indicate when the post-play delay has been done
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableStop : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableStop, PVXMLPlayable);
  public:
    virtual bool OnStart();
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableURL : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableURL, PVXMLPlayable);
  public:
    virtual PBoolean Open(PVXMLChannel & chan, const PString & arg, PINDEX delay, PINDEX repeat, PBoolean autoDelete);
    virtual bool OnStart();
  protected:
    PURL m_url;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableData : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableData, PVXMLPlayable);
  public:
    virtual PBoolean Open(PVXMLChannel & chan, const PString & arg, PINDEX delay, PINDEX repeat, PBoolean autoDelete);
    void SetData(const PBYTEArray & data);
    virtual bool OnStart();
    virtual bool OnRepeat();
  protected:
    PBYTEArray m_data;
};

//////////////////////////////////////////////////////////////////

#if P_DTMF

#include <ptclib/dtmf.h>

class PVXMLPlayableTone : public PVXMLPlayableData
{
  PCLASSINFO(PVXMLPlayableTone, PVXMLPlayableData);
  public:
    virtual PBoolean Open(PVXMLChannel & chan, const PString & arg, PINDEX delay, PINDEX repeat, PBoolean autoDelete);
  protected:
    PTones m_tones;
};

#endif // P_DTMF


//////////////////////////////////////////////////////////////////

class PVXMLPlayableCommand : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableCommand, PVXMLPlayable);
  public:
    virtual PBoolean Open(PVXMLChannel & chan, const PString & arg, PINDEX delay, PINDEX repeat, PBoolean autoDelete);
    virtual bool OnStart();
    virtual void OnStop();

  protected:
    PString m_command;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableFile : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableFile, PVXMLPlayable);
  public:
    virtual PBoolean Open(PVXMLChannel & chan, const PString & arg, PINDEX delay, PINDEX repeat, PBoolean autoDelete);
    virtual bool OnStart();
    virtual bool OnRepeat();
    virtual void OnStop();
  protected:
    PFilePath m_filePath;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableFileList : public PVXMLPlayableFile
{
  PCLASSINFO(PVXMLPlayableFileList, PVXMLPlayableFile);
  public:
    PVXMLPlayableFileList();
    virtual PBoolean Open(PVXMLChannel & chan, const PString & arg, PINDEX delay, PINDEX repeat, PBoolean autoDelete);
    virtual PBoolean Open(PVXMLChannel & chan, const PStringArray & filenames, PINDEX delay, PINDEX repeat, PBoolean autoDelete);
    virtual bool OnStart();
    virtual bool OnRepeat();
    virtual void OnStop();
  protected:
    PStringArray m_fileNames;
    PINDEX       m_currentIndex;
};

//////////////////////////////////////////////////////////////////

PQUEUE(PVXMLQueue, PVXMLPlayable);

//////////////////////////////////////////////////////////////////

class PVXMLChannel : public PDelayChannel
{
  PCLASSINFO(PVXMLChannel, PDelayChannel);
  public:
    PVXMLChannel(unsigned frameDelay, PINDEX frameSize);
    ~PVXMLChannel();

    virtual PBoolean Open(PVXMLSession * session);

    // overrides from PIndirectChannel
    virtual PBoolean IsOpen() const;
    virtual PBoolean Close();
    virtual PBoolean Read(void * buffer, PINDEX amount);
    virtual PBoolean Write(const void * buf, PINDEX len);

#if P_WAVFILE
    // new functions
    virtual PWAVFile * CreateWAVFile(const PFilePath & fn, PBoolean recording = false);
#endif

    const PString & GetMediaFormat() const { return mediaFormat; }
    PBoolean IsMediaPCM() const { return mediaFormat == "PCM-16"; }
    virtual PString AdjustWavFilename(const PString & fn);

    // Incoming channel functions
    virtual PBoolean WriteFrame(const void * buf, PINDEX len) = 0;
    virtual PBoolean IsSilenceFrame(const void * buf, PINDEX len) const = 0;

    virtual bool QueueRecordable(PVXMLRecordable * newItem);
    bool EndRecording(bool timedOut);
    bool IsRecording() const { return m_recordable != NULL; }

    // Outgoing channel functions
    virtual PBoolean ReadFrame(void * buffer, PINDEX amount) = 0;
    virtual PINDEX CreateSilenceFrame(void * buffer, PINDEX amount) = 0;
    virtual void GetBeepData(PBYTEArray &, unsigned) { }

    virtual PBoolean QueueResource(const PURL & url, PINDEX repeat= 1, PINDEX delay = 0);

    virtual PBoolean QueuePlayable(const PString & type, const PString & str, PINDEX repeat = 1, PINDEX delay = 0, PBoolean autoDelete = false);
    virtual PBoolean QueuePlayable(PVXMLPlayable * newItem);
    virtual PBoolean QueueData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);

    virtual PBoolean QueueFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0, PBoolean autoDelete = false)
    { return QueuePlayable("File", fn, repeat, delay, autoDelete); }

    virtual PBoolean QueueCommand(const PString & cmd, PINDEX repeat = 1, PINDEX delay = 0)
    { return QueuePlayable("Command", cmd, repeat, delay, true); }

    virtual void FlushQueue();
    virtual PBoolean IsPlaying() const { return m_currentPlayItem != NULL || m_playQueue.GetSize() > 0; }

    void SetPause(PBoolean pause) { m_paused = pause; }

    unsigned GetSampleFrequency() const { return m_sampleFrequency; }

    void SetSilence(unsigned msecs);

  protected:
    PVXMLSession * m_vxmlSession;

    unsigned m_sampleFrequency;
    PString mediaFormat;
    PString wavFilePrefix;

    PMutex   m_channelWriteMutex;
    PMutex   m_channelReadMutex;
    bool     m_closed;
    bool     m_paused;
    PINDEX   m_totalData;

    // Incoming audio variables
    PVXMLRecordable * m_recordable;

    // Outgoing audio variables
    PVXMLQueue      m_playQueue;
    PVXMLPlayable * m_currentPlayItem;
    PSimpleTimer    m_silenceTimer;
};


//////////////////////////////////////////////////////////////////

class PVXMLNodeHandler : public PObject
{
    PCLASSINFO(PVXMLNodeHandler, PObject);
  public:
    // Return true for process node, false to skip and move to next sibling
    virtual bool Start(PVXMLSession & /*session*/, PXMLElement & /*node*/) const { return true; }

    // Return true to move to next sibling, false to stay at this node.
    virtual bool Finish(PVXMLSession & /*session*/, PXMLElement & /*node*/) const { return true; }
};


typedef PFactory<PVXMLNodeHandler, PCaselessString> PVXMLNodeFactory;


#endif // P_VXML

#endif // PTLIB_VXML_H


// End of file ////////////////////////////////////////////////////////////////
