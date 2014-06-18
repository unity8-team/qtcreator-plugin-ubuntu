#ifndef DEMOPREVIEW_H
#define DEMOPREVIEW_H

#include<unity/scopes/PreviewQueryBase.h>
#include<unity/scopes/Result.h>
#include<unity/scopes/ActionMetadata.h>

class %DISPLAYNAME_CAPITAL%Preview : public unity::scopes::PreviewQueryBase
{
public:
    %DISPLAYNAME_CAPITAL%Preview(unity::scopes::Result const& uri, unity::scopes::ActionMetadata const& metadata);
    ~%DISPLAYNAME_CAPITAL%Preview();

    virtual void cancelled() override;
    virtual void run(unity::scopes::PreviewReplyProxy const& reply) override;
};

#endif
