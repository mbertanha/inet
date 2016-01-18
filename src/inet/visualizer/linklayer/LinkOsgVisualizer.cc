//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/OSGUtils.h"
#include "inet/linklayer/base/MACBase.h"
#include "inet/linklayer/base/MACProtocolBase.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/linklayer/LinkOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Geode>
#include <osg/LineWidth>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(LinkOsgVisualizer);

#ifdef WITH_OSG

LinkOsgVisualizer::OsgLink::OsgLink(osg::Geode *geode, int sourceModuleId, int destinationModuleId) :
    Link(sourceModuleId, destinationModuleId),
    geode(geode)
{
}

LinkOsgVisualizer::OsgLink::~OsgLink()
{
    // TODO: delete geode;
}

void LinkOsgVisualizer::addLink(std::pair<int, int> sourceAndDestination, const Link *link)
{
    LinkVisualizerBase::addLink(sourceAndDestination, link);
    auto osgLink = static_cast<const OsgLink *>(link);
    auto scene = inet::osg::getScene(visualizerTargetModule);
    scene->addChild(osgLink->geode);
}

void LinkOsgVisualizer::removeLink(const Link *link)
{
    LinkVisualizerBase::removeLink(link);
    auto osgLink = static_cast<const OsgLink *>(link);
    auto geode = osgLink->geode;
    geode->getParent(0)->removeChild(geode);
}

const LinkVisualizerBase::Link *LinkOsgVisualizer::createLink(cModule *source, cModule *destination) const
{
    auto geode = new osg::Geode();
    auto drawable = inet::osg::createLineGeometry(getPosition(source), getPosition(destination));
    geode->addDrawable(drawable);
    auto stateSet = inet::osg::createStateSet(lineColor, 1.0, false);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto lineWidth = new osg::LineWidth();
    lineWidth->setWidth(this->lineWidth);
    stateSet->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);
    geode->setStateSet(stateSet);
    return new OsgLink(geode, source->getId(), destination->getId());
}

void LinkOsgVisualizer::setAlpha(const Link *link, double alpha) const
{
    auto osgLink = static_cast<const OsgLink *>(link);
    auto geode = osgLink->geode;
    auto material = static_cast<osg::Material *>(geode->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

void LinkOsgVisualizer::setPosition(cModule *node, const Coord& position) const
{
    for (auto it : links) {
        auto link = static_cast<const OsgLink *>(it.second);
        auto geode = link->geode;
        auto geometry = static_cast<osg::Geometry *>(geode->getDrawable(0));
        auto vertices = static_cast<osg::Vec3Array *>(geometry->getVertexArray());
        if (node->getId() == it.first.first)
            vertices->at(0) = osg::Vec3d(position.x, position.y, position.z);
        else if (node->getId() == it.first.second)
            vertices->at(1) = osg::Vec3d(position.x, position.y, position.z);
        geometry->dirtyBound();
        geometry->dirtyDisplayList();
    }
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

