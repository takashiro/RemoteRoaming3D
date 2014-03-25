package com.nmlzju.roaming3d;

import java.util.*;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;

public class HotspotActivity extends FragmentActivity {

	/**
	 * The {@link android.support.v4.view.PagerAdapter} that will provide
	 * fragments for each of the sections. We use a
	 * {@link android.support.v4.app.FragmentPagerAdapter} derivative, which
	 * will keep every loaded fragment in memory. If this becomes too memory
	 * intensive, it may be best to switch to a
	 * {@link android.support.v4.app.FragmentStatePagerAdapter}.
	 */
	SectionsPagerAdapter mSectionsPagerAdapter;

	/**
	 * The {@link ViewPager} that will host the section contents.
	 */
	ViewPager mViewPager;
	
	String hotspot = null;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_hotspot);

		// Create the adapter that will return a fragment for each of the three
		// primary sections of the app.
		mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());

		// Set up the ViewPager with the sections adapter.
		mViewPager = (ViewPager) findViewById(R.id.pager);
		mViewPager.setAdapter(mSectionsPagerAdapter);
		
		Intent intent = getIntent();
		hotspot = intent.getStringExtra("hotspot");
	}

	/**
	 * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
	 * one of the sections/tabs/pages.
	 */
	public class SectionsPagerAdapter extends FragmentPagerAdapter {

		public SectionsPagerAdapter(FragmentManager fm) {
			super(fm);
		}

		@Override
		public Fragment getItem(int position) {
			// getItem is called to instantiate the fragment for the given page.
			// Return a DummySectionFragment (defined as a static inner class
			// below) with the page number as its lone argument.
			Bundle args = new Bundle();
			args.putInt("position", position);
			args.putString("hotspot", hotspot);
			
			Fragment fragment = new DummySectionFragment();
			fragment.setArguments(args);
			return fragment;
		}

		@Override
		public int getCount() {
			// Show 3 total pages.
			return 3;
		}

		@Override
		public CharSequence getPageTitle(int position) {
			switch (position) {
			case 0:
				return getString(R.string.title_text);
			case 1:
				return getString(R.string.title_gallery);
			case 2:
				return getString(R.string.title_media);
			}
			return null;
		}
	}

	/**
	 * A dummy fragment representing a section of the app, but that simply
	 * displays dummy text.
	 */
	public static class DummySectionFragment extends Fragment {
		public DummySectionFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
			Bundle args = getArguments();
			int position = args.getInt("position");
			if(position < 0 || position > 2){
				position = 0;
			}
			
			Hotspot spot = new Hotspot(args.getString("hotspot"));
			View root_view = null;
			switch(position){
			case 0:
				root_view = inflater.inflate(R.layout.fragment_hotspot_text, container, false);
				TextView title_view = (TextView) root_view.findViewById(R.id.section_title);
				TextView content_view = (TextView) root_view.findViewById(R.id.section_content);
				title_view.setText(spot.getName());
				content_view.setText(spot.getDescription());
				break;
			case 1:
				root_view = inflater.inflate(R.layout.fragment_hotspot_gallery, container, false);
				break;
			case 2:
				root_view = inflater.inflate(R.layout.fragment_hotspot_media, container, false);
				List<Map<String, Object>> data = new ArrayList<Map<String, Object>>();
				
				SimpleAdapter adapter = new SimpleAdapter(
					root_view.getContext(), data, R.layout.fragment_hotspot_media_item,
					new String[]{"title","info","img"},
					new int[]{R.id.hotspot_media_title, R.id.hotspot_media_info, R.id.hotspot_media_thumbnail}
				);
				
				((ListView) root_view).setAdapter(adapter);
				break;
			}
			return root_view;
		}
	}

}
