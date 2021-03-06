/*
 * xmpp_roster.cxx
 *
 * Extensible Messaging and Presence Protocol (XMPP) IM
 * Roster management classes
 *
 * Portable Windows Library
 *
 * Copyright (c) 2004 Reitek S.p.A.
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision: 28864 $
 * $Author: csoutheren $
 * $Date: 2013-01-12 08:51:29 +0000 (Sat, 12 Jan 2013) $
 */

#ifdef __GNUC__
#pragma implementation "xmpp_roster.h"
#endif

#include <ptlib.h>
#include <ptclib/xmpp_roster.h>

#define new PNEW


#if P_EXPAT

XMPP::Roster::Item::Item(PXMLElement * item)
  : m_IsDirty(false)
{
  if (item != NULL)
    operator=(*item);
}


XMPP::Roster::Item::Item(PXMLElement& item)
  : m_IsDirty(false)
{
  operator=(item);
}


XMPP::Roster::Item::Item(const JID& jid, ItemType type, const PString& group, const PString& name)
  : m_JID(jid),
    m_IsDirty(true)
{
  SetType(type);
  AddGroup(group);
  SetName(name.IsEmpty() ? m_JID.GetUser() : name);
}


void XMPP::Roster::Item::AddGroup(const PString& group, PBoolean dirty)
{
  if (group.IsEmpty())
    return;

  if (!m_Groups.Contains(group) && dirty)
    SetDirty();

  m_Groups.Include(group);
}


void XMPP::Roster::Item::RemoveGroup(const PString& group, PBoolean dirty)
{
  if (m_Groups.Contains(group) && dirty)
    SetDirty();

  m_Groups.Exclude(group);
}


void XMPP::Roster::Item::SetPresence(const Presence& p)
{
  JID from = p.GetFrom();
  PString res = from.GetResource();

  if (!res.IsEmpty())
    m_Presence.SetAt(res, new Presence(p));
}


XMPP::Roster::Item& XMPP::Roster::Item::operator=(const PXMLElement& item)
{
  SetJID(item.GetAttribute("jid"));
  SetName(item.GetAttribute("name"));
  if (m_Name.IsEmpty())
    SetName(m_JID.GetUser());

  PCaselessString type = item.GetAttribute("subscription");

  if (type.IsEmpty() || type == "none")
    SetType(XMPP::Roster::None);
  else if (type == "to")
    SetType(XMPP::Roster::To);
  else if (type == "from")
    SetType(XMPP::Roster::From);
  else if (type == "both")
    SetType(XMPP::Roster::Both);
  else
    SetType(XMPP::Roster::Unknown);

  PINDEX i = 0;
  PXMLElement * group;

  while ((group = item.GetElement("group", i++)) != 0)
    AddGroup(group->GetData());

  return *this;
}


PXMLElement * XMPP::Roster::Item::AsXML(PXMLElement * parent) const
{
  if (parent == NULL)
    return NULL;

  PXMLElement * item = parent->AddElement("item");
  item->SetAttribute("jid", GetJID());
  item->SetAttribute("name", GetName());

  PString s;

  switch (m_Type) {
    case XMPP::Roster::None:
      s = "none";
      break;
    case XMPP::Roster::To:
      s = "to";
      break;
    case XMPP::Roster::From:
      s = "from";
      break;
    case XMPP::Roster::Both:
      s = "both";
      break;
    default :
      break;
  }

  if (!s.IsEmpty())
    item->SetAttribute("subscrition", s);

  for (PStringSet::const_iterator it = m_Groups.begin(); it != m_Groups.end(); ++it) {
    PXMLElement * group = item->AddElement("group");
    group->AddData(*it);
  }

  return item;
}

///////////////////////////////////////////////////////

XMPP::Roster::Roster(XMPP::C2S::StreamHandler * handler)
  : m_Handler(NULL)
{
  if (handler != NULL)
    Attach(handler);
}


XMPP::Roster::~Roster()
{
}


XMPP::Roster::Item * XMPP::Roster::FindItem(const PString& jid)
{
  for (ItemList::iterator i = m_Items.begin(); i != m_Items.end(); i++) {
    if (i->GetJID() == jid)
      return &*i;
  }

  return NULL;
}


PBoolean XMPP::Roster::SetItem(Item * item, PBoolean localOnly)
{
  if (item == NULL)
    return false;

  if (localOnly) {
    Item * existingItem = FindItem(item->GetJID());

    if (existingItem != NULL)
      m_Items.Remove(existingItem);

    if (m_Items.Append(item)) {
      m_ItemChangedHandlers(*item, 0);
      m_RosterChangedHandlers(*this, 0);
      return true;
    }
    else
      return false;
  }

  XMPP::IQ iq(XMPP::IQ::Set);
  PXMLElement * query = iq.SetRootElement(XMPP::IQQueryTag());
  query->SetAttribute(XMPP::NamespaceTag(), "jabber:iq:roster");
  item->AsXML(query);

  return m_Handler->Write(iq);
}


PBoolean XMPP::Roster::RemoveItem(const PString& jid, PBoolean localOnly)
{
  Item * item = FindItem(jid);

  if (item == NULL)
    return false;

  if (localOnly) {
    m_RosterChangedHandlers(*this, false);
    m_Items.Remove(item);
    return true;
  }

  XMPP::IQ iq(XMPP::IQ::Set);
  PXMLElement * query = iq.SetRootElement(XMPP::IQQueryTag());
  query->SetAttribute(XMPP::NamespaceTag(), "jabber:iq:roster");
  item->AsXML(query)->SetAttribute("subscription", "remove");

  return m_Handler->Write(iq);
}


PBoolean XMPP::Roster::RemoveItem(Item * item, PBoolean localOnly)
{
  if (item == NULL)
    return false;

  return RemoveItem(item->GetJID(), localOnly);
}


void XMPP::Roster::Attach(XMPP::C2S::StreamHandler * handler)
{
  if (m_Handler != NULL)
    Detach();

  if (handler == NULL)
    return;

  m_Handler = handler;
  m_Handler->SessionEstablishedHandlers().Add(PCREATE_NOTIFIER(OnSessionEstablished));
  m_Handler->SessionReleasedHandlers().Add(PCREATE_NOTIFIER(OnSessionReleased));
  m_Handler->PresenceHandlers().Add(PCREATE_NOTIFIER(OnPresence));
  m_Handler->IQNamespaceHandlers("jabber:iq:roster").Add(PCREATE_NOTIFIER(OnIQ));

  if (m_Handler->IsEstablished())
    Refresh(true);
}


void XMPP::Roster::Detach()
{
  m_Items.RemoveAll();

  if (m_Handler != NULL) {
    m_Handler->SessionEstablishedHandlers().RemoveTarget(this);
    m_Handler->SessionReleasedHandlers().RemoveTarget(this);
    m_Handler->PresenceHandlers().RemoveTarget(this);
    m_Handler->IQNamespaceHandlers("jabber:iq:roster").RemoveTarget(this);
    m_Handler = 0;
  }
  m_RosterChangedHandlers(*this, 0);
}


void XMPP::Roster::Refresh(PBoolean sendPresence)
{
  if (m_Handler == NULL)
    return;

  XMPP::IQ iq(XMPP::IQ::Get);
  PXMLElement * query = iq.SetRootElement(XMPP::IQQueryTag());
  query->SetAttribute(XMPP::NamespaceTag(), "jabber:iq:roster");

  m_Handler->Write(iq);

  if (sendPresence) {
    XMPP::Presence pre;
    m_Handler->Write(pre);
  }
}


void XMPP::Roster::OnSessionEstablished(XMPP::C2S::StreamHandler&, P_INT_PTR)
{
  Refresh(true);
}


void XMPP::Roster::OnSessionReleased(XMPP::C2S::StreamHandler&, P_INT_PTR)
{
  Detach();
}


void XMPP::Roster::OnPresence(XMPP::Presence& msg, P_INT_PTR)
{
  Item * item = FindItem(msg.GetFrom());

  if (item != NULL) {
    item->SetPresence(msg);
    m_ItemChangedHandlers(*item, 0);
    m_RosterChangedHandlers(*this, 0);
  }
}


void XMPP::Roster::OnIQ(XMPP::IQ& iq, P_INT_PTR)
{
  PXMLElement * query = iq.GetElement(XMPP::IQQueryTag());

  if (PAssertNULL(query) == NULL)
    return;

  PINDEX i = 0;
  PXMLElement * item;
  bool doUpdate = false;

  while ((item = query->GetElement("item", i++)) != 0) {
    if (item->GetAttribute("subscription") == "remove")
      RemoveItem(item->GetAttribute("jid"), true);
    else
      SetItem(new XMPP::Roster::Item(item), true);
    doUpdate = true;
  }

  if (iq.GetType() == XMPP::IQ::Set) {
    iq.SetProcessed();
    
    if (!iq.GetID().IsEmpty())
      m_Handler->Send(iq.BuildResult());
  }

  if (doUpdate)
    m_RosterChangedHandlers(*this, 0);
}


#endif // P_EXPAT


// End of File ///////////////////////////////////////////////////////////////



