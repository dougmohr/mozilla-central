/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   David W. Hyatt <hyatt@netscape.com> (Original Author)
 *   Jan Varga <varga@nixcorp.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsTreeImageListener.h"
#include "nsITreeBoxObject.h"
#include "imgIContainer.h"

NS_IMPL_ISUPPORTS3(nsTreeImageListener, imgIDecoderObserver, imgIContainerObserver, nsITreeImageListener)

nsTreeImageListener::nsTreeImageListener(nsITreeBoxObject* aTree)
  : mTree(aTree),
    mInvalidationSuppressed(PR_TRUE),
    mInvalidationArea(nsnull)
{
}

nsTreeImageListener::~nsTreeImageListener()
{
  delete mInvalidationArea;
}

NS_IMETHODIMP nsTreeImageListener::OnStartDecode(imgIRequest *aRequest)
{
  return NS_OK;
}

NS_IMETHODIMP nsTreeImageListener::OnStartContainer(imgIRequest *aRequest,
                                                    imgIContainer *aImage)
{
  // Ensure the animation (if any) is started
  aImage->StartAnimation();                                                     
  return NS_OK;
}

NS_IMETHODIMP nsTreeImageListener::OnStartFrame(imgIRequest *aRequest,
                                                gfxIImageFrame *aFrame)
{
  return NS_OK;
}

NS_IMETHODIMP nsTreeImageListener::OnDataAvailable(imgIRequest *aRequest,
                                                   gfxIImageFrame *aFrame,
                                                   const nsRect *aRect)
{
  Invalidate();
  return NS_OK;
}

NS_IMETHODIMP nsTreeImageListener::OnStopFrame(imgIRequest *aRequest,
                                               gfxIImageFrame *aFrame)
{
  return NS_OK;
}

NS_IMETHODIMP nsTreeImageListener::OnStopContainer(imgIRequest *aRequest,
                                                   imgIContainer *aImage)
{
  return NS_OK;
}

NS_IMETHODIMP nsTreeImageListener::OnStopDecode(imgIRequest *aRequest,
                                                nsresult status,
                                                const PRUnichar *statusArg)
{
  return NS_OK;
}

NS_IMETHODIMP nsTreeImageListener::FrameChanged(imgIContainer *aContainer,
                                                gfxIImageFrame *newframe,
                                                nsRect * dirtyRect)
{
  Invalidate();
  return NS_OK;
}


NS_IMETHODIMP
nsTreeImageListener::AddCell(PRInt32 aIndex, nsITreeColumn* aCol)
{
  if (!mInvalidationArea) {
    mInvalidationArea = new InvalidationArea(aCol);
    mInvalidationArea->AddRow(aIndex);
  }
  else {
    InvalidationArea* currArea;
    for (currArea = mInvalidationArea; currArea; currArea = currArea->GetNext()) {
      if (currArea->GetCol() == aCol) {
        currArea->AddRow(aIndex);
        break;
      }
    }
    if (!currArea) {
      currArea = new InvalidationArea(aCol);
      currArea->SetNext(mInvalidationArea);
      mInvalidationArea = currArea;
      mInvalidationArea->AddRow(aIndex);
    }
  }

  return NS_OK;
}


void
nsTreeImageListener::Invalidate()
{
  if (!mInvalidationSuppressed) {
    for (InvalidationArea* currArea = mInvalidationArea; currArea; currArea = currArea->GetNext()) {
      // Loop from min to max, invalidating each cell that was listening for this image.
      for (PRInt32 i = currArea->GetMin(); i <= currArea->GetMax(); ++i) {
        mTree->InvalidateCell(i, currArea->GetCol());
      }
    }
  }
}

nsTreeImageListener::InvalidationArea::InvalidationArea(nsITreeColumn* aCol)
  : mCol(aCol),
    mMin(-1), // min should start out "undefined"
    mMax(0),
    mNext(nsnull)
{
}

void
nsTreeImageListener::InvalidationArea::AddRow(PRInt32 aIndex)
{
  if (mMin == -1)
    mMin = mMax = aIndex;
  else if (aIndex < mMin)
    mMin = aIndex;
  else if (aIndex > mMax)
    mMax = aIndex;
}
