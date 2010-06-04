#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cassert>

#include <vmime/vmime.hpp>
#include <vmime/platforms/posix/posixHandler.hpp>

#include "ValueLoader.h"

using namespace std;

///////////////////////////////////////////////////////////////////////
// Parameters

// The subject of the message
static string subject;

// From: <...>
static string sender;

// SMTP server settings
static string server;
static string serverUser;
static string serverPass;

// Path to the text file containing the body of the message (plain text)
static string bodyFile;

// Path to the attachment file
static string attachmentFile;

// Path to the file containing the list of subscribers
static string subscribersFile;

///////////////////////////////////////////////////////////////////////

// The list of email addresses of the subscribers.
static vector<string> subscribers;

// Text of the message body.
static string bodyText;

// Name of the attachment (file name, without path).
static string attachmentName;

///////////////////////////////////////////////////////////////////////
// Helper functions

// Return the value of the parameters specified, throw if it is missing or 
// empty.
static string
getParameter(map<string, string>& params, const string& name);

// Load the text of the message body from the file specified.
static string
loadBodyText(const string& path);

// Load the list of subscribers from the file specified.
static vector<string>
loadSubscribers(const string& path);

///////////////////////////////////////////////////////////////////////
void
loadConf(string confFile)
{
    CValueLoader valueLoader;
    valueLoader.loadValues(confFile);
    
    // store the parameters in the map - for convenience
    map<string, string> params; 
    
    ValueList tmp = valueLoader.getValueGroups().at(0);
    for (ValueList::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
        params.insert(make_pair(it->name, it->value));
    }
    
    subject         = getParameter(params, "subject");
    sender          = getParameter(params, "sender");
    server          = getParameter(params, "server");
    serverUser      = getParameter(params, "serverUser");
    serverPass      = getParameter(params, "serverPass");
    bodyFile        = getParameter(params, "bodyFile");
    attachmentFile  = getParameter(params, "attachmentFile");
    subscribersFile = getParameter(params, "subscribersFile");
    
    bodyText = loadBodyText(bodyFile);
    subscribers = loadSubscribers(subscribersFile);
    
    // Get the file name without the path.
    string::size_type pos = attachmentFile.find_last_of("/");
    if (pos == string::npos) {
        attachmentName = attachmentFile;
    } else {
        attachmentName = attachmentFile.substr(pos + 1);
    }
    
    if (attachmentName.empty()) {
        throw runtime_error(string(
            "The name of the attachment is empty."));
    }
    
    return;
}

void
sendMessage()
{
    assert(! subject.empty());
    assert(! sender.empty());
    assert(! server.empty());
    assert(! serverUser.empty());
    assert(! serverPass.empty());
    
    assert(! subscribers.empty());
    // Assume everything has been prepared.
    
    vmime::messageBuilder messageBuilder;
    
    messageBuilder.setSubject(vmime::text(subject.c_str()));
            
    messageBuilder.setExpeditor(
        vmime::mailbox(vmime::text(serverUser.c_str()), sender.c_str()));
    
    for (vector<string>::const_iterator it = subscribers.begin(); 
            it != subscribers.end(); ++it) {
        messageBuilder.getRecipients().appendAddress(
            vmime::create <vmime::mailbox>(it->c_str()));
    }
    
    messageBuilder.getTextPart()->setText(
        vmime::create <vmime::stringContentHandler>(bodyText.c_str()));
        
    // Construct the message
    vmime::ref <vmime::message> message = messageBuilder.construct();
    
    // Prepare the attachment
    vmime::ref <vmime::fileAttachment> att = 
        vmime::create <vmime::fileAttachment>(
            // path to the file
            attachmentFile.c_str(), 
            
            // content type
            vmime::mediaType("application/octet-stream"), 
            
            // description
            vmime::text(attachmentFile.c_str()) 
    );
    att->getFileInfo().setFilename(attachmentName.c_str());
    
    vmime::attachmentHelper::addAttachment(message, 
        static_cast <vmime::ref <vmime::attachment> > (att));
        
    // Create a session and send the message
    vmime::ref <vmime::net::session> session = 
        vmime::create <vmime::net::session>();
    
    // URL of the SMTP server
    string serverURL = "smtp://" + server;
    vmime::utility::url url(serverURL.c_str());
    
    // Set transport service properties
    vmime::ref <vmime::net::transport> transportService = 
        session->getTransport(url);
    transportService->setProperty("options.need-authentication", true);
    transportService->setProperty("auth.username", serverUser.c_str());
    transportService->setProperty("auth.password", serverPass.c_str());
    
    // Send the message
    transportService->connect();
    transportService->send(message);
    transportService->disconnect();
    
    return;
}

///////////////////////////////////////////////////////////////////////
static string
getParameter(map<string, string>& params, const string& name)
{
    string value = params[name];
    if (value.empty()) {
        throw runtime_error(string("Parameter \"") + name + 
            "\" is missing or has an empty value");
    }
    return value;
}

static string
loadBodyText(const string& path)
{
    // No need for binary mode as we operate on a text file anyway.
    ifstream in(path.c_str()/*, ios_base::binary*/);
    if (! in) {
        throw runtime_error(string("Failed to read the body text from ") + 
            path);
    }
    
    return string((istreambuf_iterator<char>(in)), 
        istreambuf_iterator<char>());
}

static vector<string>
loadSubscribers(const string& path)
{
    ifstream inputFile(path.c_str());
    if (!inputFile) {
        throw runtime_error(string(
            "Failed to read the list of subscribers from ") + path);
    }
    
    vector<string> subscriberList;
    
    string line;
    while (getline(inputFile, line)) {
        trimString(line);

        // If the line is blank or is a comment, skip it.
        if (line.empty() || line.at(0) == '#') {
            continue;
        }
        
        subscriberList.push_back(line);
    }
    
    if (subscriberList.empty()) {
        throw runtime_error(string(
            "No subscribers are specified in ") + path);
    }
    
    return subscriberList;
}
///////////////////////////////////////////////////////////////////////
