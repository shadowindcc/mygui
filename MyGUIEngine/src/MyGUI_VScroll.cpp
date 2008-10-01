/*!
	@file
	@author		Albert Semenov
	@date		11/2007
	@module
*/
#include "MyGUI_VScroll.h"
#include "MyGUI_InputManager.h"
#include "MyGUI_Button.h"
#include "MyGUI_WidgetSkinInfo.h"

namespace MyGUI
{

	MYGUI_RTTI_CHILD_IMPLEMENT( VScroll, Widget );

	const int SCROLL_MOUSE_WHEEL = 50; // ����������� �������� ��� ������ ����

	VScroll::VScroll(const IntCoord& _coord, Align _align, const WidgetSkinInfoPtr _info, ICroppedRectangle * _parent, IWidgetCreator * _creator, const Ogre::String & _name) :
		Widget(_coord, _align, _info, _parent, _creator, _name),
		mWidgetStart(null),
		mWidgetEnd(null),
		mWidgetTrack(null),
		mWidgetFirstPart(null),
		mWidgetSecondPart(null)
	{
		// ��� ����, ����� ������������ ������
		mScrollPage = 1;
		mScrollViewPage = 1;

		for (VectorWidgetPtr::iterator iter = mWidgetChild.begin(); iter!=mWidgetChild.end(); ++iter) {
			if ((*iter)->_getInternalString() == "Start") {
				MYGUI_DEBUG_ASSERT( ! mWidgetStart, "widget already assigned");
				mWidgetStart = (*iter)->castType<Button>();
				mWidgetStart->eventMouseButtonPressed = newDelegate(this, &VScroll::notifyMousePressed);
				mWidgetStart->eventMouseWheel = newDelegate(this, &VScroll::notifyMouseWheel);
			}
			else if ((*iter)->_getInternalString() == "End") {
				MYGUI_DEBUG_ASSERT( ! mWidgetEnd, "widget already assigned");
				mWidgetEnd = (*iter)->castType<Button>();
				mWidgetEnd->eventMouseButtonPressed = newDelegate(this, &VScroll::notifyMousePressed);
				mWidgetEnd->eventMouseWheel = newDelegate(this, &VScroll::notifyMouseWheel);
			}
			else if ((*iter)->_getInternalString() == "Track") {
				MYGUI_DEBUG_ASSERT( ! mWidgetTrack, "widget already assigned");
				mWidgetTrack = (*iter)->castType<Button>();
				mWidgetTrack->eventMouseDrag = newDelegate(this, &VScroll::notifyMouseDrag);
				mWidgetTrack->eventMouseButtonPressed = newDelegate(this, &VScroll::notifyMousePressed);
				mWidgetTrack->eventMouseButtonReleased = newDelegate(this, &VScroll::notifyMouseReleased);
				mWidgetTrack->eventMouseWheel = newDelegate(this, &VScroll::notifyMouseWheel);
				mWidgetTrack->hide();
			}
			else if ((*iter)->_getInternalString() == "FirstPart") {
				MYGUI_DEBUG_ASSERT( ! mWidgetFirstPart, "widget already assigned");
				mWidgetFirstPart = (*iter)->castType<Button>();
				mWidgetFirstPart->eventMouseButtonPressed = newDelegate(this, &VScroll::notifyMousePressed);
				mWidgetFirstPart->eventMouseWheel = newDelegate(this, &VScroll::notifyMouseWheel);
			}
			else if ((*iter)->_getInternalString() == "SecondPart") {
				MYGUI_DEBUG_ASSERT( ! mWidgetSecondPart, "widget already assigned");
				mWidgetSecondPart = (*iter)->castType<Button>();
				mWidgetSecondPart->eventMouseButtonPressed = newDelegate(this, &VScroll::notifyMousePressed);
				mWidgetSecondPart->eventMouseWheel = newDelegate(this, &VScroll::notifyMouseWheel);
			}
		}

		// slider don't have buttons
		//MYGUI_ASSERT(null != mWidgetStart, "Child Button Start not found in skin (Scroll must have Start)");
		//MYGUI_ASSERT(null != mWidgetEnd, "Child Button End not found in skin (Scroll must have End)");
		MYGUI_ASSERT(null != mWidgetTrack, "Child Button Track not found in skin (Scroll must have Track)");

		// ������ ��������
		const MapString & properties = _info->getProperties();
		MapString::const_iterator iter = properties.find("TrackRangeMargins");
		if (iter != properties.end()) {
			IntSize range = IntSize::parse(iter->second);
			mSkinRangeStart = range.width;
			mSkinRangeEnd = range.height;
		}
		else {
			mSkinRangeStart = 0;
			mSkinRangeEnd = 0;
		}
		iter = properties.find("MinTrackSize");
		if (iter != properties.end()) mMinTrackSize = utility::parseInt(iter->second);
		else mMinTrackSize = 0;
	}

	void VScroll::updateTrack()
	{
		_forcePeek(mWidgetTrack);
		// ������ ��������� � ��������
		int pos = getLineSize();

		// �������� ���� �������� ��������� ��� ����� ����
		if ((mScrollRange < 2) || (pos <= mWidgetTrack->getHeight())) {
			mWidgetTrack->hide();
			if ( null != mWidgetFirstPart ) mWidgetFirstPart->setSize(mWidgetFirstPart->getWidth(), pos/2);
			if ( null != mWidgetSecondPart ) mWidgetSecondPart->setPosition(mWidgetSecondPart->getLeft(), pos/2 + (int)mSkinRangeStart, mWidgetSecondPart->getWidth(), pos - pos/2);
			if ( pos < 0 )
			{
				//if ( null != mWidgetStart ) mWidgetStart->setSize(mWidgetStart->getWidth(), (int)mSkinRangeStart + pos/2);
				//if ( null != mWidgetEnd ) mWidgetEnd->setPosition(mWidgetEnd->getLeft(), pos/2 + (int)mSkinRangeStart, mWidgetEnd->getWidth(), mCoord.height - (pos/2 + (int)mSkinRangeStart));
			}
			else
			{
				//if ( null != mWidgetStart ) mWidgetStart->setSize(mWidgetStart->getWidth(), (int)mSkinRangeStart);
				//if ( null != mWidgetEnd ) mWidgetEnd->setPosition(mWidgetEnd->getLeft(), mCoord.height - (int)mSkinRangeEnd, mWidgetEnd->getWidth(), (int)mSkinRangeEnd);
			}
			return;
		}
		// ���� ����� �� �������
		if (false == mWidgetTrack->isShow())
		{
			mWidgetTrack->show();
			//if ( null != mWidgetStart ) mWidgetStart->setSize(mWidgetStart->getWidth(), mSkinRangeStart);
			//if ( null != mWidgetEnd ) mWidgetEnd->setPosition(mWidgetEnd->getLeft(), mCoord.height - mSkinRangeEnd, mWidgetEnd->getWidth(), mSkinRangeEnd);
		}

		// � ��������� �������
		pos = (int)(((size_t)(pos-getTrackSize()) * mScrollPosition) / (mScrollRange-1) + mSkinRangeStart);

		mWidgetTrack->setPosition(mWidgetTrack->getLeft(), pos);
		if ( null != mWidgetFirstPart )
		{
			int height = pos + mWidgetTrack->getHeight()/2 - mWidgetFirstPart->getTop();
			mWidgetFirstPart->setSize(mWidgetFirstPart->getWidth(), height);
		}
		if ( null != mWidgetSecondPart )
		{
			int top = pos + mWidgetTrack->getHeight()/2;
			int height = mWidgetSecondPart->getHeight() + mWidgetSecondPart->getTop() - top;
			mWidgetSecondPart->setPosition(mWidgetSecondPart->getLeft(), top, mWidgetSecondPart->getWidth(), height);
		}
	}

	void VScroll::TrackMove(int _left, int _top)
	{
		const IntPoint & point = InputManager::getInstance().getLastLeftPressed();

		// ����������� ������� �������
		int start = mPreActionRect.top + (_top - point.top);
		if (start < (int)mSkinRangeStart) start = (int)mSkinRangeStart;
		else if (start > (mCoord.height - (int)mSkinRangeEnd - mWidgetTrack->getHeight())) start = (mCoord.height - (int)mSkinRangeEnd - mWidgetTrack->getHeight());
		if (mWidgetTrack->getTop() != start) mWidgetTrack->setPosition(mWidgetTrack->getLeft(), start);

		// ����������� ��������� ��������������� �������
		// ���� ��� �������
		int pos = start - (int)mSkinRangeStart + (getLineSize() - getTrackSize()) / (((int)mScrollRange-1) * 2);
		// ����������� ��������� �������� � ���������
		pos = pos * (int)(mScrollRange-1) / (getLineSize() - getTrackSize());

		// ��������� �� ������ � ���������
		if (pos < 0) pos = 0;
		else if (pos >= (int)mScrollRange) pos = (int)mScrollRange - 1;
		if (pos == (int)mScrollPosition) return;

		mScrollPosition = pos;
		// �������� �������
		eventScrollChangePosition(this, (int)mScrollPosition);
	}

	void VScroll::notifyMousePressed(WidgetPtr _sender, int _left, int _top, MouseButton _id)
	{
		// ��������������� ������� ����� ����� ��� ����
		eventMouseButtonPressed(this, _left, _top, _id);

		if (MB_Left != _id) return;

		if (_sender == mWidgetStart) {
			// ����������� ��������
			if (mScrollPosition == 0) return;

			// ����������� ��������� ���������
			if (mScrollPosition > mScrollPage) mScrollPosition -= mScrollPage;
			else mScrollPosition = 0;

			// ���������
			eventScrollChangePosition(this, (int)mScrollPosition);
			updateTrack();

		}
		else if (_sender == mWidgetEnd) {
			// ������������ ��������
			if ( (mScrollRange < 2) || (mScrollPosition >= (mScrollRange-1)) ) return;

			// ����������� ��������� ���������
			if ((mScrollPosition + mScrollPage) < (mScrollRange-1)) mScrollPosition += mScrollPage;
			else mScrollPosition = mScrollRange - 1;

			// ���������
			eventScrollChangePosition(this, (int)mScrollPosition);
			updateTrack();

		}
		else if (_sender == mWidgetFirstPart) {
			// ����������� ��������
			if (mScrollPosition == 0) return;

			// ����������� ��������� ���������
			if (mScrollPosition > mScrollViewPage) mScrollPosition -= mScrollViewPage;
			else mScrollPosition = 0;

			// ���������
			eventScrollChangePosition(this, (int)mScrollPosition);
			updateTrack();

		}
		else if (_sender == mWidgetSecondPart) {
			// ������������ ��������
			if ( (mScrollRange < 2) || (mScrollPosition >= (mScrollRange-1)) ) return;

			// ����������� ��������� ���������
			if ((mScrollPosition + mScrollViewPage) < (mScrollRange-1)) mScrollPosition += mScrollViewPage;
			else mScrollPosition = mScrollRange - 1;

			// ���������
			eventScrollChangePosition(this, (int)mScrollPosition);
			updateTrack();

		}
		else {
			mPreActionRect.left = _sender->getLeft();
			mPreActionRect.top = _sender->getTop();
		}
	}

	void VScroll::notifyMouseReleased(WidgetPtr _sender, int _left, int _top, MouseButton _id)
	{
		updateTrack();
	}

	void VScroll::notifyMouseDrag(WidgetPtr _sender, int _left, int _top)
	{
		TrackMove(_left, _top);
	}

	void VScroll::setScrollRange(size_t _range)
	{
		if (_range == mScrollRange) return;
		mScrollRange = _range;
		mScrollPosition = (mScrollPosition < mScrollRange) ? mScrollPosition : 0;
		updateTrack();
	}

	void VScroll::setScrollPosition(size_t _position)
	{
		if (_position == mScrollPosition) return;
		if (_position >= mScrollRange) _position = 0;
		mScrollPosition = _position;
		updateTrack();
	}

	void VScroll::setSize(const IntSize& _size)
	{
		Widget::setSize(_size);
		// ��������� ����
		updateTrack();
	}

	void VScroll::setPosition(const IntCoord& _coord)
	{
		Widget::setPosition(_coord);
		// ��������� ����
		updateTrack();
	}

	void VScroll::setTrackSize(size_t _size)
	{
		mWidgetTrack->setSize(mWidgetTrack->getWidth(), ((int)_size < (int)mMinTrackSize)? (int)mMinTrackSize : (int)_size);
		updateTrack();
	}

	int VScroll::getTrackSize()
	{
		return mWidgetTrack->getHeight();
	}

	int VScroll::getLineSize()
	{
		return mCoord.height - (int)(mSkinRangeStart + mSkinRangeEnd);
	}

	void VScroll::_onMouseWheel(int _rel)
	{
		notifyMouseWheel(null, _rel);
		Widget::_onMouseWheel(_rel);
	}

	void VScroll::notifyMouseWheel(WidgetPtr _sender, int _rel)
	{
		if (mScrollRange < 2) return;

		int offset = mScrollPosition;
		if (_rel < 0) offset += SCROLL_MOUSE_WHEEL;
		else  offset -= SCROLL_MOUSE_WHEEL;

		if (offset < 0) offset = 0;
		else if (offset > (int)(mScrollRange - 1)) offset = mScrollRange - 1;

		if (offset != mScrollPosition) {
			mScrollPosition = offset;
			// ���������
			eventScrollChangePosition(this, (int)mScrollPosition);
			updateTrack();
		}
	}

} // namespace MyGUI
