// Aseprite
// Copyright (C) 2001-2016  David Capello
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/commands/command.h"
#include "app/context_access.h"
#include "app/modules/gui.h"
#include "doc/image.h"
#include "doc/layer.h"

namespace app {

using namespace ui;

class LayerVisibilityCommand : public Command {
public:
  LayerVisibilityCommand();
  Command* clone() const override { return new LayerVisibilityCommand(*this); }

protected:
  bool onEnabled(Context* context) override;
  bool onChecked(Context* context) override;
  void onExecute(Context* context) override;
};

LayerVisibilityCommand::LayerVisibilityCommand()
  : Command("LayerVisibility",
            "Layer Visibility",
            CmdRecordableFlag)
{
}

bool LayerVisibilityCommand::onEnabled(Context* context)
{
  return context->checkFlags(ContextFlags::ActiveDocumentIsWritable |
                             ContextFlags::HasActiveLayer);
}

bool LayerVisibilityCommand::onChecked(Context* context)
{
  const ContextReader reader(context);
  const SelectedLayers& selLayers = reader.site()->selectedLayers();
  bool visible = false;
  for (auto layer : selLayers)
  {
    if(layer && layer->isVisible())
      visible = true;
  }
  return visible;
}

void LayerVisibilityCommand::onExecute(Context* context)
{
  ContextWriter writer(context);
  const SelectedLayers& selLayers = writer.site()->selectedLayers();

  bool anyVisible = false;
  for (auto layer : selLayers)
  {
    if(layer->isVisible())
      anyVisible = true;
  }
  for(auto layer : selLayers)
  {
    layer->setVisible(!anyVisible);
  }

  update_screen_for_document(writer.document());
}

Command* CommandFactory::createLayerVisibilityCommand()
{
  return new LayerVisibilityCommand;
}

} // namespace app
