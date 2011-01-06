#include "stdafx.h"

#include <string>
#include <sstream>

#include "RprNode.hxx"

int RprPropertyNode::childCount()
{
	return 0;
}

void RprPropertyNode::addChild(RprNode *node)
{}

void RprPropertyNode::toReaper(std::ostringstream &oss, int indent)
{
	std::string strIndent(indent, ' ');
	oss << strIndent.c_str() << getValue().c_str();
	oss << std::endl;
}

RprNode* RprPropertyNode::getChild(int index)
{
	return NULL;
}

const std::string& RprNode::getValue()
{
	return mValue;
}

void RprNode::setValue(const std::string& value)
{
	mValue = value;
}

RprNode *RprNode::getParent()
{
	return mParent;
}

void RprNode::setParent(RprNode *parent)
{
	mParent = parent;
}

RprParentNode::~RprParentNode()
{
	for(std::vector<RprNode *>::iterator i = mChildren.begin();
		i != mChildren.end();
		i++) {
		delete *i;
	}
}

RprParentNode::RprParentNode(const char *value)
{
	setValue(value);
}

RprNode *RprParentNode::getChild(int index)
{
	return mChildren.at(index);
}
	
void RprParentNode::addChild(RprNode *node)
{
	node->setParent(this);
	mChildren.push_back(node);
}

void RprParentNode::addChild(RprNode *node, int index)
{
	node->setParent(this);
	mChildren.insert(mChildren.begin() + index, node);
}

int RprParentNode::childCount()
{
	return mChildren.size();
}

void RprParentNode::removeChild(int index)
{
	RprNode *child = mChildren.at(index);
	mChildren.erase(mChildren.begin() + index);
	delete child;
}

static std::string getTrimmedLine(std::istringstream &iss)
{
	char lineBuffer[256];
	iss.getline(lineBuffer, 256);
	std::string line;
	int offset = 0;
	if(lineBuffer[0] == '\n')
		offset++;
	while(lineBuffer[offset] == ' ') offset++;
	line = std::string(lineBuffer + offset);
	return line;
}

void RprParentNode::toReaper(std::ostringstream &oss, int indent)
{
	std::string strIndent(indent, ' ');
	oss << strIndent.c_str() << "<";
	oss << getValue().c_str() << std::endl;
	for(std::vector<RprNode *>::iterator i = mChildren.begin();
		i != mChildren.end();
		i++) {
		(*i)->toReaper(oss, 0);
	}
	oss << strIndent.c_str() << ">" << std::endl;
}

RprNode *RprParentNode::clone()
{
	RprParentNode *node = new RprParentNode(getValue().c_str());
	for(std::vector<RprNode *>::iterator i = mChildren.begin();
		i != mChildren.end();
		i++) {
		RprNode *child = *i;
		node->addChild(child->clone());
	}
	return node;
}

std::string RprNode::toReaper()
{
	std::ostringstream oss;
	toReaper(oss, 0);
	return oss.str();
}

static RprNode *addNewChildNode(RprNode *node, const std::string &value)
{
	RprNode *newNode = new RprParentNode(value.c_str()); 
	node->addChild(newNode);
	return newNode;
}

static void addNewPropertyNode(RprNode *node, const std::string &property)
{
	node->addChild(new RprPropertyNode(property));
}

RprPropertyNode::RprPropertyNode(const std::string &value)
{
	setValue(value);
}

void RprPropertyNode::removeChild(int index)
{}

RprNode *RprPropertyNode::clone()
{
	RprPropertyNode *node = new RprPropertyNode(getValue());
	return node;
}

RprNode *RprParentNode::createItemStateTree(const char *itemState)
{
	if(itemState == NULL)
		return NULL;
	std::istringstream iss(itemState);
	RprParentNode *parentNode = new RprParentNode("ROOT");
	RprNode *currentNode = parentNode;
	while(!iss.eof()) {
		std::string line = getTrimmedLine(iss);
		if(line.empty())
			continue;
		if(line[0] == '<')
			currentNode = addNewChildNode(currentNode, line.substr(1));
		else if(line[0] == '>')
			currentNode = currentNode->getParent();
		else
			addNewPropertyNode(currentNode, line);
	}
	RprNode *itemNode = parentNode->getChild(0);

	itemNode->setParent(NULL);
	parentNode->mChildren.clear();
	delete parentNode;
	return itemNode;
}

